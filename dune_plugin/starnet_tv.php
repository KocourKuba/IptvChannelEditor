<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/hashed_array.php';
require_once 'lib/tv/abstract_tv.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_channel.php';

///////////////////////////////////////////////////////////////////////////

class StarnetPluginTv extends AbstractTv
{
    public function __construct(DefaultConfig $config)
    {
        parent::__construct(AbstractTv::MODE_CHANNELS_N_TO_M, $config, false);
    }

    public function get_fav_icon_url()
    {
        return ViewsConfig::FAV_CHANNEL_GROUP_ICON_PATH;
    }

    ///////////////////////////////////////////////////////////////////////
    public function set_setup_screen($setup_screen)
    {
        $this->SettingsScreen = $setup_screen;
    }

    public function get_setup_screen()
    {
        return isset($this->SettingsScreen) ? $this->SettingsScreen : false;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function load_channels(&$plugin_cookies)
    {
        $channels_list = $this->get_channel_list_url($plugin_cookies);
        hd_print("Channels list: $channels_list");

        try {
            $doc = file_get_contents($channels_list, true);
            if($doc == false)
            {
                hd_print("File not exist! $channels_list");
                throw new Exception('File not exist');
            }
        } catch (Exception $e) {
            hd_print("Can't fetch channel_list, alternative copy used.");
            return;
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

        // read embedded access info
        $plugin_cookies->ott_key_local = "";
        $plugin_cookies->subdomain_local = "";
        if (isset($xml->channels_setup)
            && !empty($xml->channels_setup->access_key)
            && !empty($xml->channels_setup->access_domain)
           )
        {
            hd_print("Overriding access settings found in playlist: " . $channels_list);
            $plugin_cookies->ott_key_local = strval($xml->channels_setup->access_key);
            $plugin_cookies->subdomain_local = strval($xml->channels_setup->access_domain);
        }

        // Create channels and groups
        $this->channels = new HashedArray();
        $this->groups = new HashedArray();

        // Favorites group
        if ($this->is_favorites_supported()) {
            $this->groups->put(new FavoritesGroup($this,
                ViewsConfig::FAV_CHANNEL_GROUP_ID,
                ViewsConfig::FAV_CHANNEL_GROUP_CAPTION,
                ViewsConfig::FAV_CHANNEL_GROUP_ICON_PATH));
        }

        // All channels group
        $this->groups->put(new AllChannelsGroup($this,
            ViewsConfig::ALL_CHANNEL_GROUP_CAPTION,
            ViewsConfig::ALL_CHANNEL_GROUP_ICON_PATH));

        // read category
        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                $error_string = "Error: unexpected node '" . $xml_tv_category->getName() . "'. Expected: 'tv_category'";
                hd_print($error_string);
                throw new Exception($error_string);
            }

            $this->groups->put(new DefaultGroup(strval($xml_tv_category->id),
                strval($xml_tv_category->caption),
                strval($xml_tv_category->icon_url)));
        }

        // Read channels
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print("Error: unexpected node '" . $xml_tv_channel->getName() . "'. Expected: 'tv_channel'");
                continue;
            }

            // ignore disabled channel
            if (isset($xml_tv_channel->disabled)) continue;

            // make substitute template
            if (isset($xml_tv_channel->channel_id))
                $streaming_url = str_replace('{ID}', $xml_tv_channel->channel_id, $this->config->MEDIA_URL_TEMPLATE);
            else
                $streaming_url = strval($xml_tv_channel->streaming_url);

            // calculate unique id from url hash
            $id = hash("crc32", $streaming_url);
            if ($this->channels->has($id)) {
                // added or existing channel
                $channel = $this->channels->get($id);
            }
            else
            {
                $channel = new StarnetChannel(
                    $id,
                    strval($xml_tv_channel->caption),
                    strval($xml_tv_channel->icon_url),
                    intval($xml_tv_channel->archive),
                    $streaming_url,
                    intval($xml_tv_channel->number),
                    intval($xml_tv_channel->tvg_id),
                    intval($xml_tv_channel->epg_id),
                    intval($xml_tv_channel->protected),
                    intval($xml_tv_channel->timeshift_hours));

                $this->channels->put($channel);
            }

            // Read category id from channel
            if (isset($xml_tv_channel->tv_category_id)) {
                // new format
                $tv_category_id = intval($xml_tv_channel->tv_category_id);
                $group = $this->groups->get($tv_category_id);

                // Link group and channel.
                $channel->add_group($group);
                $group->add_channel($channel);

                // only new format support favorites
                if ($xml_tv_channel->favorite && $this->is_favorites_supported()) {
                    hd_print("Add to favorite $id ($xml_tv_channel->caption)");
                    $this->change_tv_favorites(PLUGIN_FAVORITES_OP_ADD, $id, $plugin_cookies);
                }
            } else if (isset($xml_tv_channel->tv_categories)) {
                // old format
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
            } else {
                hd_print("Error: Category undefined for channel $id ($xml_tv_channel->caption) !");
            }
        }

        hd_print("Loaded: channels: " . $this->channels->size() .", groups: " . $this->groups->size());
    }

    public function get_tv_stream_url($playback_url, &$plugin_cookies)
    {
        return $playback_url;
    }

    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        try {
            $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
            $channel = $this->get_channel($channel_id);
            if ($channel->is_protected() && $protect_code !== $pass_sex) {
                throw new Exception('Wrong password');
            }
        } catch (Exception $ex) {
            return '';
        }

        $url = $this->config->AdjustStreamUri($plugin_cookies, $archive_ts, $channel->get_streaming_url());

        $plugin_cookies->subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : 'dunehd.iptvspy.net';

        if (!empty($plugin_cookies->subdomain_local) && !empty($plugin_cookies->ott_key_local)) {
            $url = str_replace('{SUBDOMAIN}', $plugin_cookies->subdomain_local, $url);
            $url = str_replace('{UID}', $plugin_cookies->ott_key_local, $url);
        } else {
            $url = str_replace('{SUBDOMAIN}', $plugin_cookies->subdomain, $url);
            $url = str_replace('{UID}', $plugin_cookies->ott_key, $url);
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

        try {
            $channel = $this->get_channel($channel_id);
        } catch (Exception $ex) {
            hd_print("Can't get channel with ID: $channel_id");
            return array();
        }

        // get personal time shift for channel
        $time_shift = $channel->get_timeshift_hours() * 3600;
        $tvg_id = intval($channel->get_tvg_id());
        $epg_id = intval($channel->get_epg_id());
        $epg_date = gmdate("Ymd", $day_start_ts);

        if (!is_dir($this->config->EPG_CACHE_DIR)) {
           mkdir($this->config->EPG_CACHE_DIR);
        }

        $epg = array();
        $cache_file = $this->config->EPG_CACHE_DIR . $this->config->EPG_CACHE_FILE . $channel_id . "_" . $day_start_ts;
        if (file_exists($cache_file)) {
            $epg = unserialize(file_get_contents($cache_file));
        } else {
            // if all tvg & epg empty no need to fetch data
            if ($tvg_id == 0 && $epg_id == 0)
                return $epg;

            try {
                // tvguide used as backup of ott-play epg source
                // sharavoz used as backup of arlekino epg source
                $id = $epg_id;
                $provider = $this->config->EPG_PROVIDER;
                $epg = $this->get_epg($provider, $id, $epg_date, $day_start_ts);
            } catch (Exception $ex) {
                try {
                    hd_print("Can't fetch EPG ID from ($provider): $id DATE: $epg_date");
                    $provider = $this->config->TVG_PROVIDER;
                    hd_print("using second ($provider)");
                    $epg = $this->get_epg($provider, $id, $epg_date, $day_start_ts);
                } catch (Exception $ex) {
                    hd_print("Can't fetch EPG ID from backup ($provider): $id DATE: $epg_date");
                    return $epg;
                }
            }
        }

        if (count($epg) > 0) {
            ksort($epg, SORT_NUMERIC);
            file_put_contents($cache_file, serialize($epg));
        }

        $epg_result = array();
        $start = 0;
        $end = 0;
        foreach ($epg as $time => $value) {
            $tm =  $time + $time_shift;
            $end = $tm;
            if ($start == 0)
                $start = $tm;

            $epg_result[] = new DefaultEpgItem(
                    str_replace(array_keys($replace), $replace, strval($value["title"])),
                    str_replace(array_keys($replace), $replace, strval($value["desc"])),
                    intval($tm), -1);
        }

        return new EpgIterator($epg_result, $start, $end);
    }

    /**
     * @throws Exception
     */
    protected function get_epg($provider, $epg_id, $epg_date, $day_start_ts)
    {
        $epg = array();

        switch ($provider) {
            case 'ott-play':
                $url = sprintf($this->config->EPG_URL_FORMAT, $epg_id);
                break;
            case 'arlekino':
                $url = sprintf($this->config->EPG_URL_FORMAT, $epg_id, $epg_date);
                break;
            case 'teleguide':
            case 'sharavoz':
                $url = sprintf($this->config->TVG_URL_FORMAT, $epg_id, $epg_date);
                break;
            default:
                break;
        }

        if (empty($url)) {
            hd_print("Unknown provider: $provider");
            return $epg;
        }

        hd_print("provider:$provider epg url: $url");
        switch ($provider) {
            case 'ott-play':
            case 'arlekino':
            case 'sharavoz':
                // json parse
                $doc = HD::http_get_document($url);
                $epg = $this->parse_epg_json($doc, $day_start_ts);
                break;
            case 'teleguide':
                // html parse
                // tvguide.info time in GMT+3 (moscow time)
                // $timezone_suffix = date('T');
                $doc = HD::http_get_document($url);
                $e_time = strtotime("$epg_date, 0300 GMT+3");
                preg_match_all('|<div id="programm_text">(.*?)</div>|', $doc, $keywords);
                foreach ($keywords[1] as $qid) {
                    $qq = strip_tags($qid);
                    preg_match_all('|(\d\d:\d\d)&nbsp;(.*?)&nbsp;(.*)|', $qq, $keyw);
                    $time = $keyw[1][0];
                    $u_time = strtotime("$epg_date $time GMT+3");
                    $last_time = ($u_time < $e_time) ? $u_time + 86400  : $u_time ;
                    $epg[$last_time]["title"] = str_replace("&nbsp;", " ", $keyw[2][0]);
                    $epg[$last_time]["desc"] = str_replace("&nbsp;", " ", $keyw[3][0]);
                }
                break;
			default:
				break;
        }

        return $epg;
    }

    protected function parse_epg_json($doc, $day_start_ts)
    {
        $epg = array();
        $ch_data = json_decode(ltrim($doc, chr(239) . chr(187) . chr(191)));
        // time in UTC
        $epg_date_start = strtotime('-1 hour', $day_start_ts);
        $epg_date_end = strtotime('+1 day', $day_start_ts);
        foreach ($ch_data->epg_data as $channel) {
            if ($channel->time >= $epg_date_start and $channel->time < $epg_date_end) {
                $epg[$channel->time]['title'] = $channel->name;
                $epg[$channel->time]['desc'] = $channel->descr;
            }
        }
        return $epg;
    }
}

///////////////////////////////////////////////////////////////////////////
?>
