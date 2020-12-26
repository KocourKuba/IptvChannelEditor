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

    public function load_channels(&$plugin_cookies)
    {
        try {
            $channels_list = isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : DemoConfig::CHANNEL_LIST_URL;
            hd_print("Channels list: $channels_list");
            $doc = file_get_contents($channels_list, true);
            if($doc == false)
            {
                hd_print("File not exist! $channels_list");
                throw new Exception('File not exist');
            }

        } catch (Exception $e) {
            hd_print("Can't fetch channel_list, alternative copy used.");
            $doc = file_get_contents(DemoConfig::CHANNEL_LIST_URL, true);
        }

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
            $this->groups->put(new FavoritesGroup($this, '__favorites', DemoConfig::FAV_CHANNEL_GROUP_CAPTION, DemoConfig::FAV_CHANNEL_GROUP_ICON_PATH));
        }

        $this->groups->put(new AllChannelsGroup($this, DemoConfig::ALL_CHANNEL_GROUP_CAPTION, DemoConfig::ALL_CHANNEL_GROUP_ICON_PATH));

        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                hd_print("Error: unexpected node '" . $xml_tv_category->getName() . "'. Expected: 'tv_category'");
                throw new Exception('Invalid XML document');
            }

            $this->groups->put(new DefaultGroup(strval($xml_tv_category->id), strval($xml_tv_category->caption), strval($xml_tv_category->icon_url)));
        }

        $id = 0;
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print("Error: unexpected node '" . $xml_tv_channel->getName() . "'. Expected: 'tv_channel'");
                throw new Exception('Invalid XML document');
            }

            if (isset($xml_tv_channel->disabled)) continue;

            $cid = $id . "_" . $xml_tv_channel->epg_id . "_" . $xml_tv_channel->tvg_id;
            $id++;
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 0; //буферизация

            $timeshift_hours = 0;
            $channel =
                new DemoChannel(
                    strval($cid),
                    strval($xml_tv_channel->caption),
                    strval($xml_tv_channel->icon_url),
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
                    hd_print("Error: unexpected node '" . $xml_tv_cat_id->getName() . "'. Expected: 'tv_category_id'");
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
        return $playback_url;
    }

    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';

        try {
            $channel = $this->get_channel($channel_id);
            if ($channel->is_protected() && $protect_code !== $pass_sex) {
                throw new Exception('Wrong password');
            }

            $url = $channel->get_streaming_url();
        } catch (Exception $ex) {
            return '';
        }

        $plugin_cookies->subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : 'dunehd.iptvspy.net';
        $url =  str_replace('{SUBDOMAIN}', $plugin_cookies->subdomain, $url);
        $url =  str_replace('{UID}', $plugin_cookies->ott_key, $url);

        if (intval($archive_ts) > 0) {
            $now_ts = intval(time());
            $url .= "?utc=$archive_ts&lutc=$now_ts";
        }

        return $url;
    }

    public function get_day_epg_iterator($channel_id, $day_start_ts, &$plugin_cookies)
    {
        $replace = array(
            '&#196;' => 'Г„',
            '&#228;' => 'Г¤',
            '&#214;' => 'Г–',
            '&#220;' => 'Гњ',
            '&#223;' => 'Гџ',
            '&#246;' => 'Г¶',
            '&#252;' => 'Гј',
            '&#39;'  => "'",
            '&quot;' => '"',
            '&#257;' => 'ā',
            '&#258;' => 'Ă',
            '&#268;' => 'Č',
            '&#326;' => 'ņ',
            '&#327;' => 'Ň',
            '&#363;' => 'ū',
            '&#362;' => 'Ū',
            '&#352;' => 'Š',
            '&#353;' => 'š',
            '&#382;' => 'ž',
            '&#275;' => 'ē',
            '&#276;' => 'Ĕ',
            '&#298;' => 'Ī',
            '&#299;' => 'ī',
            '&#291;' => 'ģ',
            '&#311;' => 'ķ',
            '&#316;' => 'ļ',
        );

        list($garb, $epg_id, $tvg_id) = preg_split('/_/', $channel_id);

        $epg_date = gmdate("Ymd", $day_start_ts);

        $epg = array();
        if (!is_dir(DemoConfig::EPG_CACHE_DIR)) {
           mkdir(DemoConfig::EPG_CACHE_DIR);
        }

        $cache_file = DemoConfig::EPG_CACHE_DIR . DemoConfig::EPG_CACHE_FILE . $epg_id . "_" . $tvg_id . "_" . $day_start_ts;
        if (file_exists($cache_file)) {
            $doc = file_get_contents($cache_file);
            $epg = unserialize($doc);
        } else {
            $type = 'json'; // ott-play.com
            if ($tvg_id == 0) {
                try {
                    $doc = HD::http_get_document(sprintf(DemoConfig::EPG_URL_FORMAT, $epg_id));
                }
                catch (Exception $ex) {
                    hd_print("Can't fetch EPG ID: $epg_id DATE: $epg_date");
                    return array();
                }
            } else {
                $type = 'html'; // teleguide.info
                try {
                    $doc = HD::http_get_document(sprintf(DemoConfig::EPG_URL_FORMAT2, $tvg_id, $epg_date));
                }
                catch (Exception $ex) {
                    hd_print("Can't fetch TVG ID: $tvg_id DATE: $epg_date");
                    return array();
                }
            }
            if ($type === 'json') {
                // time in UTC
                $ch_data = json_decode(ltrim($doc, chr(239) . chr(187) . chr(191)));
                $epg_date_new = strtotime('-1 hour', $day_start_ts);
                $epg_date_end = strtotime('+1 day', $day_start_ts);
                foreach ($ch_data->epg_data as $channel) {
                    if ($channel->time >= $epg_date_new and $channel->time < $epg_date_end) {
                        $epg[$channel->time]['title'] = $channel->name;
                        $epg[$channel->time]['desc'] = $channel->descr;
                    }
                }
            }
            else
            {
                // tvguide.info time in GMT+3 (moscow time)
                // $timezone_suffix = date('T');
                $e_time = strtotime("$epg_date, 0300 GMT+3");
                preg_match_all('|<div id="programm_text">(.*?)</div>|', $doc, $keywords);
                foreach ($keywords[1] as $key => $qid) {
                    $qq = strip_tags($qid);
                    preg_match_all('|(\d\d:\d\d)&nbsp;(.*?)&nbsp;(.*)|', $qq, $keyw);
                    $time = $keyw[1][0];
                    $u_time = strtotime("$epg_date $time GMT+3");
                    $last_time = ($u_time < $e_time) ? $u_time + 86400  : $u_time ;
                    $epg[$last_time]["title"] = str_replace("&nbsp;", " ", $keyw[2][0]);
                    $epg[$last_time]["desc"] = str_replace("&nbsp;", " ", $keyw[3][0]);
                }
            }
        }

        if (count($epg) > 0) {
            ksort($epg, SORT_NUMERIC);
            file_put_contents($cache_file, serialize($epg));
        }

        $epg_result = array();
        $epg_shift = isset($plugin_cookies->epg_shift) ? $plugin_cookies->epg_shift : '0';
        $start = 0;
        $end = 0;
        foreach ($epg as $time => $value) {
            $tm =  $time + $epg_shift;
            $end = $tm;
            if ($start == 0)
                $start = $tm;

            $epg_result[] = new DefaultEpgItem(
                    str_replace(array_keys($replace), $replace, strval($value["title"])),
                    str_replace(array_keys($replace), $replace, strval($value["desc"])),
                    intval($tm), intval(-1));
        }

        return new EpgIterator($epg_result, $start, $end);
    }
}

///////////////////////////////////////////////////////////////////////////
?>
