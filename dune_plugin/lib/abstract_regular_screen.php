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

require_once 'abstract_screen.php';

abstract class Abstract_Regular_Screen extends Abstract_Screen
{
    /**
     * @return array[]
     */
    abstract public function get_folder_views();

    ///////////////////////////////////////////////////////////////////////
    // Screen interface

    /**
     * @inheritDoc
     */
    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        hd_debug_print($media_url, true);

        $index = $this->get_folder_view_index($plugin_cookies);
        $folder_views = $this->get_folder_views();
        $folder_view = $folder_views[$index];
        $folder_view[PluginRegularFolderView::actions] = $this->get_action_map($media_url, $plugin_cookies);
        $folder_view[PluginRegularFolderView::initial_range] = $this->get_folder_range($media_url, 0, $plugin_cookies);
        $folder_view[PluginRegularFolderView::timer] = $this->get_timer($media_url, $plugin_cookies);

        return array(
            PluginFolderView::multiple_views_supported => (count($folder_views) > 1 ? 1 : 0),
            PluginFolderView::archive => null,
            PluginFolderView::view_kind => PLUGIN_FOLDER_VIEW_REGULAR,
            PluginFolderView::data => $folder_view
        );
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @inheritDoc
     */
    public function get_next_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);

        $idx = $this->get_folder_view_index($plugin_cookies);
        $folder_views = $this->get_folder_views();
        if (++$idx >= count($folder_views)) {
            $idx = 0;
        }

        $folder_views_index = "screen." . static::ID . ".view_idx";
        $plugin_cookies->{$folder_views_index} = $idx;

        return $this->get_folder_view($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    private function get_folder_view_index(&$plugin_cookies)
    {
        hd_debug_print(null, true);

        $folder_views_index = "screen." . static::ID . ".view_idx";
        $idx = isset($plugin_cookies->{$folder_views_index}) ? $plugin_cookies->{$folder_views_index} : 0;

        $folder_views = $this->get_folder_views();
        $cnt = count($folder_views);
        if ($idx < 0) {
            $idx = 0;
        } else if ($idx >= $cnt) {
            $idx = $cnt - 1;
        }

        return $idx;
    }

    /**
     * @param array $items
     * @param int $from_ndx
     * @param int $total
     * @param bool $more_items_available
     * @return array
     */
    public function create_regular_folder_range($items, $from_ndx = 0, $total = -1, $more_items_available = false)
    {
        if ($total === -1) {
            $total = $from_ndx + count($items);
        }

        if ($from_ndx >= $total) {
            $from_ndx = $total;
            $items = array();
        } else if ($from_ndx + count($items) > $total) {
            array_splice($items, $total - $from_ndx);
        }

        return array
        (
            PluginRegularFolderRange::total => (int)$total,
            PluginRegularFolderRange::more_items_available => $more_items_available,
            PluginRegularFolderRange::from_ndx => (int)$from_ndx,
            PluginRegularFolderRange::count => count($items),
            PluginRegularFolderRange::items => $items
        );
    }
}
