<?php
///////////////////////////////////////////////////////////////////////////

require_once 'controls_screen.php';
require_once 'control_factory.php';

abstract class AbstractControlsScreen implements ControlsScreen
{
    private $id;

    ///////////////////////////////////////////////////////////////////////

    protected function __construct($id)
    {
        $this->id = $id;

        UserInputHandlerRegistry::get_instance()->register_handler($this);
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_handler_id()
    {
        return $this->id.'_handler';
    }

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

    ///////////////////////////////////////////////////////////////////////

    protected function add_vgap(&$defs, $vgap)
    {
        ControlFactory::add_vgap($defs, $vgap);
    }

    protected function add_label(&$defs, $title, $text)
    {
        ControlFactory::add_label($defs, $title, $text);
    }

    protected function add_button(&$defs, $name, $title, $caption, $width = 0)
    {
        ControlFactory::add_button($defs, $this, null, $name, $title, $caption, $width);
    }

    public function add_image_button(&$defs, $name, $title, $caption, $image, $width = 0)
    {
        $push_action = UserInputHandlerRegistry::create_action($this, $name, null);
        $push_action['params']['action_type'] = 'apply';

        $defs[] = array
        (
            GuiControlDef::name => $name,
            GuiControlDef::title => $title,
            GuiControlDef::kind => GUI_CONTROL_BUTTON,
            GuiControlDef::specific_def => array
            (
                GuiButtonDef::caption => '',
                GuiButtonDef::width => $width,
                GuiButtonDef::push_action => $push_action,
            ),
        );

        ControlFactory::add_vgap($defs, -65);
        ControlFactory::add_smart_label($defs, null,  "<gap width=15/><icon>$image</icon><gap width=20/><text dy='-2'>$caption</text>");
    }

    protected function add_close_dialog_button(&$defs, $caption, $width)
    {
        ControlFactory::add_close_dialog_button($defs,
            $caption, $width);
    }

    protected function add_close_dialog_and_apply_button(&$defs, $name, $caption, $width)
    {
        ControlFactory::add_close_dialog_and_apply_button($defs,
            $this, null,
            $name, $caption, $width);
    }

    protected function add_text_field(&$defs,
                                      $name, $title, $initial_value,
                                      $numeric, $password, $has_osk, $always_active, $width,
                                      $need_confirm = false, $need_apply = false)
    {
        ControlFactory::add_text_field($defs, $this, null,
            $name, $title, $initial_value,
            $numeric, $password, $has_osk, $always_active, $width,
            $need_confirm, $need_apply);
    }

    protected function add_combobox(&$defs,
                                    $name, $title, $initial_value, $value_caption_pairs, $width,
                                    $need_confirm = false, $need_apply = false)
    {
        ControlFactory::add_combobox($defs, $this, null,
            $name, $title, $initial_value, $value_caption_pairs, $width,
            $need_confirm, $need_apply);
    }
}
