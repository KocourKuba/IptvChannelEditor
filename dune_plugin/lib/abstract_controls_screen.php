<?php
///////////////////////////////////////////////////////////////////////////

require_once 'screen.php';

abstract class AbstractControlsScreen implements Screen
{
    private $id;
    protected $plugin;

    ///////////////////////////////////////////////////////////////////////

    protected function __construct($id, DefaultDunePlugin $plugin)
    {
        $this->id = $id;
        $this->plugin = $plugin;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    abstract public function get_control_defs(MediaURL $media_url, &$plugin_cookies);

    public function get_id()
    {
        return $this->id;
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        $defs = $this->get_control_defs($media_url, $plugin_cookies);

        $folder_view = array
        (
            PluginControlsFolderView::defs => $defs,
            PluginControlsFolderView::initial_sel_ndx => -1,
        );

        return array
        (
            PluginFolderView::multiple_views_supported => false,
            PluginFolderView::archive => null,
            PluginFolderView::view_kind => PLUGIN_FOLDER_VIEW_CONTROLS,
            PluginFolderView::data => $folder_view,
        );
    }
}
