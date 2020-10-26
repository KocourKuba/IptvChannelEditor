<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/hashed_array.php';
require_once 'lib/tv/abstract_tv.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_channel.php';
require_once 'starnet_config.php';

///////////////////////////////////////////////////////////////////////////

class DemoTv extends AbstractTv
{
    public function __construct()
    {
        parent::__construct(
            AbstractTv::MODE_CHANNELS_N_TO_M,
            DemoConfig::TV_FAVORITES_SUPPORTED,
            false);
    }

    public function get_fav_icon_url()
    {
        return DemoConfig::FAV_CHANNEL_GROUP_ICON_PATH;
    }

    ///////////////////////////////////////////////////////////////////////
    public function set_setup_screen($setup_screen)
    {
        $this->SettingsScreen = $setup_screen;
    }

    public function get_setup_screen()
    {
        if (!isset($this->SettingsScreen))
            return false;

        return $this->SettingsScreen;
    }

    ///////////////////////////////////////////////////////////////////////

    protected function load_channels(&$plugin_cookies)
    {
        try {

            $doc = file_get_contents(DemoConfig::CHANNEL_LIST_URL, true);
        } catch (Exception $e) {
            hd_print("Can't fetch channel_list2, alternative copy used.");
            $doc = file_get_contents(DemoConfig::CHANNEL_LIST_URL2, true);
        }

        if (is_null($doc))
            throw new Exception('Can not fetch playlist');

        $xml = simplexml_load_string($doc);

        if ($xml === false) {
            hd_print("Error: can not parse XML document.");
            hd_print("XML-text: $doc.");
            throw new Exception('Illegal XML document');
        }

        if ($xml->getName() !== 'tv_info') {
            hd_print("Error: unexpected node '" . $xml->getName() . "'. Expected: 'tv_info'");
            throw new Exception('Invalid XML document');
        }

        $this->channels = new HashedArray();
        $this->groups = new HashedArray();

        if ($this->is_favorites_supported()) {
            $this->groups->put(
                new FavoritesGroup(
                    $this,
                    '__favorites',
                    DemoConfig::FAV_CHANNEL_GROUP_CAPTION,
                    DemoConfig::FAV_CHANNEL_GROUP_ICON_PATH));
        }

        $this->groups->put(
            new AllChannelsGroup(
                $this,
                DemoConfig::ALL_CHANNEL_GROUP_CAPTION,
                DemoConfig::ALL_CHANNEL_GROUP_ICON_PATH));

//	   $channels_id_parsed = array();
//	   $doc = HD::http_get_document(sprintf(DemoConfig::EPG_ID_FILE_URL, '.bomba.', '/dune/'));
//         $channels_id_parsed = unserialize($doc);

        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                hd_print("Error: unexpected node '" . $xml_tv_category->getName() .
                    "'. Expected: 'tv_category'");
                throw new Exception('Invalid XML document');
            }

            $this->groups->put(
                new DefaultGroup(
                    strval($xml_tv_category->id),
                    strval($xml_tv_category->caption),
                    strval($xml_tv_category->icon_url)));
        }
        $i = 0;
        $gid = 0;
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print("Error: unexpected node '" . $xml_tv_channel->getName() .
                    "'. Expected: 'tv_channel'");
                throw new Exception('Invalid XML document');
            }

//            $id_key = md5(strtolower(str_replace(array("\r", "\n", "\"", " "), '', $xml_tv_channel->caption)));
//            $id = array_key_exists($id_key,$channels_id_parsed) ? $channels_id_parsed[$id_key] : 1050 + $i;
//            $id = $xml_tv_channel->epg_id;
            $id = $xml_tv_channel->tvg_id;
            $cid = $gid . "_" . $id;
            $i++;
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 0; //буферизация


            $timeshift_hours = 0;
            $channel =
                new DemoChannel(
                    strval($cid),
                    strval($xml_tv_channel->caption),
                    strval($xml_tv_channel->icon_url),
//                    $have_archive,								//jun
                    intval($xml_tv_channel->archive),
                    strval($xml_tv_channel->streaming_url),
                    intval($xml_tv_channel->number),
                    intval($xml_tv_channel->num_past_epg_days),
                    intval($xml_tv_channel->num_future_epg_days),
                    intval($xml_tv_channel->protected),
                    $timeshift_hours, $buf_time);

            $this->channels->put($channel);

            foreach ($xml_tv_channel->tv_categories->children() as $xml_tv_cat_id) {
                if ($xml_tv_cat_id->getName() !== 'tv_category_id') {
                    hd_print("Error: unexpected node '" . $xml_tv_cat_id->getName() .
                        "'. Expected: 'tv_category_id'");
                    throw new Exception('Invalid XML document');
                }

                $tv_category_id = intval($xml_tv_cat_id);

                $group = $this->groups->get($tv_category_id);

                // Link group and channel.

                $channel->add_group($group);
                $group->add_channel($channel);
            }
        }
    }


    public function get_tv_stream_url($playback_url, &$plugin_cookies)
    {
        $plugin_cookies->subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : 'dunehd.iptvspy.net';
        return str_replace('{SUBDOMAIN}/iptv/{UID}', $plugin_cookies->subdomain . '/iptv/' . $plugin_cookies->ott_key, $playback_url);
        //	return str_replace('{UID}', $plugin_cookies->ott_key, $playback_url);
    }

    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        $url = $this->get_channel($channel_id)->get_streaming_url();
        $now_ts = intval(time());

        if (intval($archive_ts) > 0)
            $url .= "?utc=$archive_ts&lutc=$now_ts";

//        $url =  str_replace('iptv1.zargacum', $plugin_cookies->country_server, $url);
//                hd_print("cursor--->>> url: $url");

        $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';


        $nado = $this->get_channel($channel_id)->is_protected();
        if ($nado) {
            if ($protect_code !== $pass_sex) $url = '';  //Children protection code = 0000
        }
        return $url;
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////


    public function get_day_epg_iterator($channel_id, $day_start_ts, &$plugin_cookies)
    {

        list($garb, $channel_id) = preg_split('/_/', $channel_id);

        if ($channel_id >= 1299 and $channel_id <= 1300)
            return array();

        $epg_shift = isset($plugin_cookies->epg_shift) ? $plugin_cookies->epg_shift : '0';
        $epg_date = date("Ymd", $day_start_ts);
        $epg = array();

        if (file_exists("/tmp/edem_channel_" . $channel_id . "_" . $day_start_ts)) {
            $doc = file_get_contents("/tmp/edem_channel_" . $channel_id . "_" . $day_start_ts);
            $epg = unserialize($doc);
        } else {
            try {
                $doc = HD::http_get_document(sprintf(DemoConfig::EPG_URL_FORMAT2, $channel_id, $epg_date));
            } catch (Exception $e) {
                hd_print("Can't fetch EPG ID:$id DATE:$epg_date");
                return array();
            }
            $e_time = strtotime("$epg_date, 0500 EEST");
            preg_match_all('|<div id="programm_text">(.*?)</div>|', $doc, $keywords);
            $last_time = 0;
            foreach ($keywords[1] as $key => $qid) {
                $qq = strip_tags($qid);
                preg_match_all('|(\d\d:\d\d)&nbsp;(.*?)&nbsp;(.*)|', $qq, $keyw);
                $time = $keyw[1][0];
                $u_time = strtotime("$epg_date $time EEST") - 3600;
                //$last_time = ($u_time < $e_time) ? $u_time + 86400  : $u_time ;
                $last_time = ($u_time > $last_time) ? $u_time : $u_time + 86400;
                $epg[$last_time]["name"] = str_replace("&nbsp;", " ", $keyw[2][0]);
                $epg[$last_time]["desc"] = str_replace("&nbsp;", " ", $keyw[3][0]);
            }
            file_put_contents("/tmp/edem_channel_" . $channel_id . "_" . $day_start_ts, serialize($epg));
        }
        $epg_result = array();

        ksort($epg, SORT_NUMERIC);
        foreach ($epg as $time => $value) {
            $epg_result[] =
                new DefaultEpgItem(
                    strval($value["name"]),
                    strval($value["desc"]),
                    intval($time + $epg_shift),
                    intval(-1));
        }

        return
            new EpgIterator(
                $epg_result,
                $day_start_ts,
                $day_start_ts + 100400);
    }
}

///////////////////////////////////////////////////////////////////////////
?>
