﻿<?php
///////////////////////////////////////////////////////////////////////////

require_once 'tv.php';
require_once 'default_group.php';
require_once 'favorites_group.php';
require_once 'all_channels_group.php';

abstract class AbstractTv implements Tv
{
    const MODE_CHANNELS_1_TO_N = false;
    const MODE_CHANNELS_N_TO_M = true;

    ///////////////////////////////////////////////////////////////////////
    public $SettingsScreen;

    private $mode;
    private $favorites_supported;
    private $playback_url_is_stream_url;

    protected $channels;
    protected $groups;

    ///////////////////////////////////////////////////////////////////////

    protected function __construct($mode,
                                   $favorites_supported,
                                   $playback_url_is_stream_url)
    {
        $this->mode = $mode;
        $this->favorites_supported = $favorites_supported;
        $this->playback_url_is_stream_url = $playback_url_is_stream_url;
    }

    ///////////////////////////////////////////////////////////////////////

    public function is_favorites_supported()
    {
        return $this->favorites_supported;
    }

    public function get_all_channel_group_id()
    {
        return '__all_channels';
    }

    ///////////////////////////////////////////////////////////////////////

    public function unload_channels()
    {
        $this->channels = null;
        $this->groups = null;
    }

    public abstract function load_channels(&$plugin_cookies);

    public function ensure_channels_loaded(&$plugin_cookies)
    {
        if (!isset($this->channels))
            $this->load_channels($plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_channels()
    {
        return $this->channels;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_channel($channel_id)
    {
        $c = $this->channels->get($channel_id);

        if (is_null($c))
            throw new Exception("Unknown channel: $channel_id");

        return $c;
    }

    public function get_channel_list_url($plugin_cookies)
    {
        return isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : PluginConfig::CHANNEL_LIST_URL;
    }
    ///////////////////////////////////////////////////////////////////////

    public function get_groups()
    {
        return $this->groups;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_group($group_id)
    {
        $g = $this->groups->get($group_id);

        if (is_null($g))
            throw new Exception("Unknown group: $group_id");

        return $g;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    public function get_tv_info(MediaURL $media_url, &$plugin_cookies)
    {
        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : DemoSetupScreen::EPG_FONTSIZE_DEF_VALUE;
        $this->ensure_channels_loaded($plugin_cookies);

        $channels = array();

        foreach ($this->get_channels() as $c) {
            $group_id_arr = array();

            if ($this->mode == self::MODE_CHANNELS_N_TO_M)
                $group_id_arr[] = $this->get_all_channel_group_id();

            foreach ($c->get_groups() as $g)
                $group_id_arr[] = $g->get_id();

            array_push($channels,
                array
                (
                    PluginTvChannel::id => $c->get_id(),
                    PluginTvChannel::caption => $c->get_title(),
                    PluginTvChannel::group_ids => $group_id_arr,
                    PluginTvChannel::icon_url => $c->get_icon_url(),
                    PluginTvChannel::number => $c->get_number(),

                    PluginTvChannel::have_archive => $c->has_archive(),
                    PluginTvChannel::is_protected => $c->is_protected(),

                    // set default epg range
                    PluginTvChannel::past_epg_days => intval(isset($plugin_cookies->epg_prev) ? $plugin_cookies->epg_prev : $c->get_past_epg_days()),
                    PluginTvChannel::future_epg_days => intval(isset($plugin_cookies->epg_next) ? $plugin_cookies->epg_next : $c->get_future_epg_days()),

                    PluginTvChannel::archive_past_sec => intval($c->get_archive_past_sec()),
                    PluginTvChannel::archive_delay_sec => intval($c->get_archive_delay_sec()),

                    // Buffering time
                    PluginTvChannel::buffering_ms => intval(isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 0),
                    PluginTvChannel::timeshift_hours => intval($c->get_timeshift_hours()),

                    PluginTvChannel::playback_url_is_stream_url => $this->playback_url_is_stream_url,
                ));
        }

        $groups = array();

        foreach ($this->get_groups() as $g) {
            if ($g->is_favorite_channels())
                continue;

            if ($this->mode == self::MODE_CHANNELS_1_TO_N &&
                $g->is_all_channels()) {
                continue;
            }

            array_push($groups,
                array
                (
                    PluginTvGroup::id => $g->get_id(),
                    PluginTvGroup::caption => $g->get_title(),
                    PluginTvGroup::icon_url => $g->get_icon_url()
                ));
        }

        $is_favorite_group = isset($media_url->is_favorites);
        $initial_group_id = strval($media_url->group_id);
        $initial_is_favorite = 0;

        if ($is_favorite_group) {
            $initial_group_id = null;
            $initial_is_favorite = 1;
        }

        if ($this->mode == self::MODE_CHANNELS_1_TO_N &&
            $initial_group_id === $this->get_all_channel_group_id()) {
            $initial_group_id = null;
        }

        $fav_channel_ids = null;
        if ($this->is_favorites_supported())
            $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);

        $archive = $this->get_archive($media_url);
        $archive_def = is_null($archive) ? null : $archive->get_archive_def();

        return array
        (
            PluginTvInfo::show_group_channels_only => $this->mode,

            PluginTvInfo::groups => $groups,
            PluginTvInfo::channels => $channels,

            PluginTvInfo::favorites_supported => $this->is_favorites_supported(),
            PluginTvInfo::favorites_icon_url => strval($this->get_fav_icon_url()),

            PluginTvInfo::initial_channel_id => strval($media_url->channel_id),
            PluginTvInfo::initial_group_id => $initial_group_id,

            PluginTvInfo::initial_is_favorite => $initial_is_favorite,
            PluginTvInfo::favorite_channel_ids => $fav_channel_ids,

            PluginTvInfo::archive => $archive_def,

            PluginTvInfo::epg_font_size => $epg_font_size,
        );
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_tv_stream_url($playback_url, &$plugin_cookies)
    {
        throw new Exception('Error: get_tv_stream_url() is not supported.');
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        $this->ensure_channels_loaded($plugin_cookies);

        return $this->get_channel($channel_id)->get_streaming_url();
    }

    ///////////////////////////////////////////////////////////////////////

    protected abstract function get_day_epg_iterator($channel_id, $day_start_ts, &$plugin_cookies);

    ///////////////////////////////////////////////////////////////////////

    public function get_day_epg($channel_id, $day_start_ts, &$plugin_cookies)
    {
        $epg = array();

        foreach ($this->get_day_epg_iterator($channel_id, $day_start_ts, $plugin_cookies) as $e) {
            $epg[] =
                array
                (
                    PluginTvEpgProgram::start_tm_sec => $e->get_start_time(),
                    PluginTvEpgProgram::end_tm_sec => $e->get_finish_time(),
                    PluginTvEpgProgram::name => $e->get_title(),
                    PluginTvEpgProgram::description => $e->get_description()
                );
        }

        return $epg;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    public function is_favorite_channel_id($channel_id, $plugin_cookies)
    {
        if (!$this->is_favorites_supported())
            return false;

        $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);

        $k = array_search($channel_id, $fav_channel_ids);

        return $k !== false;
    }

    public function change_tv_favorites($fav_op_type, $channel_id, &$plugin_cookies)
    {
        $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);

        switch ($fav_op_type) {
            case PLUGIN_FAVORITES_OP_ADD:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== true)
                    array_push($fav_channel_ids, $channel_id);
                break;
            case PLUGIN_FAVORITES_OP_REMOVE:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false)
                    unset ($fav_channel_ids[$k]);
                break;
            case PLUGIN_FAVORITES_OP_MOVE_UP:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false && $k !== 0) {
                    $t = $fav_channel_ids[$k - 1];
                    $fav_channel_ids[$k - 1] = $fav_channel_ids[$k];
                    $fav_channel_ids[$k] = $t;
                }
                break;
            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false && $k !== count($fav_channel_ids) - 1) {
                    $t = $fav_channel_ids[$k + 1];
                    $fav_channel_ids[$k + 1] = $fav_channel_ids[$k];
                    $fav_channel_ids[$k] = $t;
                }
                break;
        }

        $this->set_fav_channel_ids($plugin_cookies, $fav_channel_ids);

        return ActionFactory::invalidate_folders(
            array(TvFavoritesScreen::get_media_url_str()));
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_fav_channel_ids($plugin_cookies)
    {
        $fav_channel_ids = array();

        $favorites = $this->get_fav_cookie($plugin_cookies);
        if (isset($plugin_cookies->{ $favorites }))
            $fav_channel_ids = preg_split('/,/', $plugin_cookies->{ $favorites });

        return array_unique($fav_channel_ids);
    }

    ///////////////////////////////////////////////////////////////////////

    public function set_fav_channel_ids($plugin_cookies, $ids)
    {
        $plugin_cookies->{ $this->get_fav_cookie($plugin_cookies) } = join(',', array_unique($ids));
    }

    ///////////////////////////////////////////////////////////////////////
    // Archive.

    public function get_archive(MediaURL $media_url)
    {
        return null;
    }

    ///////////////////////////////////////////////////////////////////////
    // Hooks.

    public function folder_entered(MediaURL $media_url, &$plugin_cookies)
    { /* Nop */
    }

    // Hook for adding special group items.
    public function add_special_groups(&$items)
    { /* Nop */
    }

    public function get_fav_cookie($plugin_cookies)
    {
        return 'favorite_channels_' . hash('crc32', $this->get_channel_list_url($plugin_cookies));
    }
}

///////////////////////////////////////////////////////////////////////////
?>
