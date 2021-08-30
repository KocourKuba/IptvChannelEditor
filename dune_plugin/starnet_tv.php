<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/hashed_array.php';
require_once 'lib/default_config.php';
require_once 'lib/tv/abstract_tv.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'lib/epg_parser.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_channel.php';

///////////////////////////////////////////////////////////////////////////

class StarnetPluginTv extends AbstractTv
{
    public $config = null;

    public function __construct(DefaultConfig $config)
    {
        parent::__construct(AbstractTv::MODE_CHANNELS_N_TO_M, false);
        $this->config = $config;
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

    public function is_favorites_supported()
    {
        return $this->config->GET_TV_FAVORITES_SUPPORTED();
    }

    public function get_channel_list_url($plugin_cookies)
    {
        return isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : $this->config->GET_CHANNEL_LIST_URL();
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
            if (isset($xml_tv_channel->channel_id)) {
                $channel_id = strval($xml_tv_channel->channel_id);
                $streaming_url = str_replace('{ID}', $xml_tv_channel->channel_id, $this->config->GET_MEDIA_URL_TEMPLATE());
            } else {
                $channel_id ='';
                $streaming_url = strval($xml_tv_channel->streaming_url);
            }
            // calculate unique id from url hash
            $id = hash("crc32", $streaming_url);
            if ($this->channels->has($id)) {
                // added or existing channel
                $channel = $this->channels->get($id);
            }
            else
            {
                // https not supported for old players
                $icon_url = str_replace('https', 'https', strval($xml_tv_channel->icon_url));
                $channel = new StarnetChannel(
                    $id,
                    $channel_id,
                    strval($xml_tv_channel->caption),
                    $icon_url,
                    $streaming_url,
                    intval($xml_tv_channel->archive),
                    strval($xml_tv_channel->epg_id),
                    strval($xml_tv_channel->tvg_id),
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
            $url = str_replace('{TOKEN}', $plugin_cookies->ott_key_local, $url);
        } else {
            $url = str_replace('{SUBDOMAIN}', $plugin_cookies->subdomain, $url);
            $url = str_replace('{TOKEN}', $plugin_cookies->ott_key, $url);
        }

        //hd_print("get_tv_playback_url: $url");
        return $url;
    }

    public function get_day_epg_iterator($channel_id, $day_start_ts, &$plugin_cookies)
    {
        try {
            $channel = $this->get_channel($channel_id);
        } catch (Exception $ex) {
            hd_print("Can't get channel with ID: $channel_id");
            return array();
        }

        if (DefaultConfig::LoadCachedEPG($channel, $day_start_ts, $epg) === false) {
            $epg = DefaultConfig::GetEPG($channel, $day_start_ts);
        }

        hd_print("Loaded " . count($epg) . " EPG entries");

        $epg_result = array();
        $start = 0;
        $end = 0;
        // get personal time shift for channel
        $time_shift = $channel->get_timeshift_hours() * 3600;
        foreach ($epg as $time => $value) {
            $tm =  $time + $time_shift;
            $end = $tm;
            if ($start == 0)
                $start = $tm;

            $epg_result[] = new DefaultEpgItem($value['title'], $value['desc'], intval($tm), -1);
        }

        return new EpgIterator($epg_result, $start, $end);
    }
}

///////////////////////////////////////////////////////////////////////////
