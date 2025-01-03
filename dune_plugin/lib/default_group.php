<?php
/**
 * The MIT License (MIT)
 *
 * @Author: sharky72 (https://github.com/KocourKuba)
 * Original code from DUNE HD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

require_once 'group.php';
require_once 'json_serializer.php';

class Default_Group extends Json_Serializer implements Group
{
    /////////////////////////////////////////////////////////////////////////////
    // views constants
    const DEFAULT_GROUP_ICON = 'plugin_file://icons/categories/default_group.png';

    const ALL_CHANNEL_GROUP_CAPTION = 'plugin_all_channels';
    const ALL_CHANNEL_GROUP_ICON = 'plugin_file://icons/categories/all.png';

    const FAV_CHANNEL_GROUP_CAPTION = 'plugin_favorites';
    const FAV_CHANNEL_GROUP_ICON = 'plugin_file://icons/categories/fav.png';

    const HISTORY_GROUP_CAPTION = 'plugin_history';
    const HISTORY_GROUP_ICON = 'plugin_file://icons/categories/history.png';

    const CHANGED_CHANNELS_GROUP_CAPTION = 'plugin_changed';
    const CHANGED_CHANNELS_GROUP_ICON = 'plugin_file://icons/categories/changed_channels.png';

    const VOD_GROUP_CAPTION = 'plugin_vod';
    const VOD_GROUP_ICON = "plugin_file://icons/categories/vod.png";

    /**
     * @var string
     */
    protected $_id;

    /**
     * @var string
     */
    protected $_title;

    /**
     * @var string
     */
    protected $_icon_url;

    /**
     * @var boolean
     */
    protected $_adult;

    /**
     * @var boolean
     */
    protected $_disabled;

    /**
     * @var Hashed_Array
     */
    protected $_channels;

    /**
     * @var string
     */
    protected $_order_settings;

    /**
     * @var Default_Dune_Plugin
     */
    private $plugin;

    /**
     * @param $plugin
     * @param string $id
     * @param string $title
     * @param string|null $icon_url
     * @param string|null $order_prefix
     */
    public function __construct($plugin, $id, $title, $icon_url = null, $order_prefix = null)
    {
        $this->plugin = $plugin;

        if (is_null($title)) {
            $title = $id;
        }

        $this->_id = $id;
        $this->_title = $title;
        $this->_icon_url = $icon_url;
        $this->_adult = false;
        $this->_disabled = false;
        $this->_order_settings = is_null($order_prefix) ? null : ($order_prefix . $this->_id);

        $this->_channels = new Hashed_Array();
    }

    /**
     * @inheritDoc
     */
    public function get_id()
    {
        return $this->_id;
    }

    /**
     * @inheritDoc
     */
    public function get_title()
    {
        return $this->_title;
    }

    /**
     * @inheritDoc
     */
    public function get_icon_url()
    {
        return $this->_icon_url;
    }

    /**
     * @inheritDoc
     */
    public function set_icon_url($icon_url)
    {
        $this->_icon_url = $icon_url;
    }

    /**
     * @inheritDoc
     */
    public function is_special_group()
    {
        return in_array($this->_id, array(ALL_CHANNEL_GROUP_ID, FAVORITES_GROUP_ID, HISTORY_GROUP_ID, VOD_GROUP_ID));
    }

    /**
     * @inheritDoc
     */
    public function is_adult_group()
    {
        return $this->_adult;
    }

    /**
     * @inheritDoc
     */
    public function set_adult($adult)
    {
        $this->_adult = $adult;
    }

    /**
     * @inheritDoc
     */
    public function is_disabled()
    {
        if ($this->_id === ALL_CHANNEL_GROUP_ID) {
            return !$this->plugin->get_bool_parameter(PARAM_SHOW_ALL);
        }

        if ($this->_id === FAVORITES_GROUP_ID) {
            return !$this->plugin->get_bool_parameter(PARAM_SHOW_FAVORITES);
        }

        if ($this->_id === HISTORY_GROUP_ID) {
            return !$this->plugin->get_bool_parameter(PARAM_SHOW_HISTORY);
        }

        if ($this->_id === VOD_GROUP_ID) {
            return !$this->plugin->get_bool_parameter(PARAM_SHOW_VOD) || $this->plugin->config->get_feature(Plugin_Constants::VOD_ENGINE) === "None";
        }

        return $this->_disabled;
    }

    /**
     * @inheritDoc
     */
    public function set_disabled($disabled)
    {
        $this->_disabled = $disabled;
    }

    /**
     * @inheritDoc
     */
    public function get_group_channels()
    {
        return $this->_channels;
    }

    /**
     * @inheritDoc
     */
    public function get_items_order()
    {
        if (is_null($this->_order_settings)) {
            return new Ordered_Array();
        }

        return $this->plugin->get_parameter($this->_order_settings, new Ordered_Array());
    }

    /**
     * @inheritDoc
     */
    public function set_items_order($order)
    {
        if ($this->_order_settings !== null) {
            $this->plugin->set_parameter($this->_order_settings, $order);
        }
    }

    /**
     * @inheritDoc
     */
    public function get_media_url_str()
    {
        if ($this->_id === FAVORITES_GROUP_ID) {
            return Starnet_Tv_Favorites_Screen::get_media_url_string($this->get_id());
        }

        if ($this->_id === HISTORY_GROUP_ID) {
            return Starnet_TV_History_Screen::get_media_url_string($this->get_id());
        }

        return Starnet_Tv_Channel_List_Screen::get_media_url_string($this->get_id());
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Methods

    /**
     * @param Channel $channel
     */
    public function add_channel(Channel $channel)
    {
        $this->_channels->set($channel->get_id(), $channel);
        if ($this->_order_settings !== null && $this->_id !== ALL_CHANNEL_GROUP_ID && !$channel->is_disabled()) {
            $order = $this->get_items_order();
            $order->add_item($channel->get_id());
            $this->set_items_order($order);
        }
    }
}
