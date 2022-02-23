<?php

require_once 'action_factory.php';

class ControlFactory
{
    public static function add_vgap(&$defs, $vgap)
    {
        $defs[] = array
        (
            GuiControlDef::kind => GUI_CONTROL_VGAP,
            GuiControlDef::specific_def => array(GuiVGapDef::vgap => $vgap),
        );
    }

    public static function add_label(&$defs, $title, $text, $vgap_after = false)
    {
        $defs[] = array
        (
            GuiControlDef::name => '',
            GuiControlDef::title => $title,
            GuiControlDef::kind => GUI_CONTROL_LABEL,
            GuiControlDef::specific_def => array(GuiLabelDef::caption => $text),
        );

        if ($vgap_after !== false) {
            self::add_vgap($defs, $vgap_after);
        }
    }

    public static function add_smart_label(&$defs, $title, $text)
    {
        $defs[] = array (
            GuiControlDef::name => '',
            GuiControlDef::title => $title,
            GuiControlDef::kind => GUI_CONTROL_LABEL,
            GuiControlDef::specific_def => array(GuiLabelDef::caption => $text),
            GuiControlDef::params => array('smart' => true),
        );
    }

    public static function add_multiline_label(&$defs, $title, $text, $num_lines = 2)
    {
        $defs[] = array(
            GuiControlDef::name => '',
            GuiControlDef::title => $title,
            GuiControlDef::kind => GUI_CONTROL_LABEL,
            GuiControlDef::specific_def => array(GuiLabelDef::caption => $text,),
            GuiControlDef::params => array('smart' => false, 'max_lines' => $num_lines),
        );
    }

    public static function add_button(&$defs, $handler, $add_params,
                                      $name, $title, $caption, $width,  $caption_centered = false)
    {
        $push_action = UserInputHandlerRegistry::create_action($handler, $name, $add_params);
        $push_action['params']['action_type'] = 'apply';

        $defs[] = array
        (
            GuiControlDef::name => $name,
            GuiControlDef::title => $title,
            GuiControlDef::kind => GUI_CONTROL_BUTTON,
            GuiControlDef::params =>array('button_caption_centered' => $caption_centered),
            GuiControlDef::specific_def => array
            (
                GuiButtonDef::caption => $caption,
                GuiButtonDef::width => $width,
                GuiButtonDef::push_action => $push_action,
            ),
        );
    }

    public static function add_image_button(&$defs, $handler, $add_params, $name, $title, $caption, $image, $width = 0)
    {
        $push_action = UserInputHandlerRegistry::create_action($handler, $name, $add_params);
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

        self::add_vgap($defs, -65);
        self::add_smart_label($defs, null,  "<gap width=15/><icon>$image</icon><gap width=20/><text dy='-2'>$caption</text>");
    }

    public static function add_close_dialog_button(&$defs, $caption, $width)
    {
        $defs[] = array
        (
            GuiControlDef::name => 'close',
            GuiControlDef::title => null,
            GuiControlDef::kind => GUI_CONTROL_BUTTON,
            GuiControlDef::specific_def => array
            (
                GuiButtonDef::caption => $caption,
                GuiButtonDef::width => $width,
                GuiButtonDef::push_action => ActionFactory::close_dialog(),
            ),
        );
    }

    public static function add_close_dialog_and_apply_button(&$defs, $handler, $add_params,
                                                             $name, $caption, $width)
    {
        $push_action = UserInputHandlerRegistry::create_action($handler, $name, $add_params);
        $push_action['params']['action_type'] = 'apply';

        $defs[] = array
        (
            GuiControlDef::name => $name,
            GuiControlDef::title => null,
            GuiControlDef::kind => GUI_CONTROL_BUTTON,
            GuiControlDef::specific_def => array
            (
                GuiButtonDef::caption => $caption,
                GuiButtonDef::width => $width,
                GuiButtonDef::push_action => ActionFactory::close_dialog_and_run($push_action),
            ),
        );
    }

    public static function add_custom_close_dialog_and_apply_buffon(&$defs, $name, $caption, $width, $action)
    {
        $defs[] = array
        (
            GuiControlDef::name => $name,
            GuiControlDef::title => null,
            GuiControlDef::kind => GUI_CONTROL_BUTTON,
            GuiControlDef::specific_def => array
            (
                GuiButtonDef::caption => $caption,
                GuiButtonDef::width => $width,
                GuiButtonDef::push_action => ActionFactory::close_dialog_and_run($action),
            ),
        );
    }

    public static function add_text_field(&$defs,
                                          $handler, $add_params,
                                          $name, $title, $initial_value,
                                          $numeric, $password, $has_osk, $always_active, $width,
                                          $need_confirm = false, $need_apply = false)
    {
        $apply_action = null;
        if ($need_apply) {
            $apply_action = UserInputHandlerRegistry::create_action($handler, $name, $add_params);
            $apply_action['params']['action_type'] = 'apply';
        }

        $confirm_action = null;
        if ($need_confirm) {
            $confirm_action = UserInputHandlerRegistry::create_action($handler, $name, $add_params);
            $confirm_action['params']['action_type'] = 'confirm';
        }

        $defs[] = array
        (
            GuiControlDef::name => $name,
            GuiControlDef::title => $title,
            GuiControlDef::kind => GUI_CONTROL_TEXT_FIELD,
            GuiControlDef::specific_def => array
            (
                GuiTextFieldDef::initial_value => (string)$initial_value,
                GuiTextFieldDef::numeric => (int)$numeric,
                GuiTextFieldDef::password => (int)$password,
                GuiTextFieldDef::has_osk => (int)$has_osk,
                GuiTextFieldDef::always_active => (int)$always_active,
                GuiTextFieldDef::width => (int)$width,
                GuiTextFieldDef::apply_action => $apply_action,
                GuiTextFieldDef::confirm_action => $confirm_action,
            ),
        );
    }

    public static function add_combobox(&$defs,
                                        $handler, $add_params,
                                        $name, $title, $initial_value, $value_caption_pairs, $width,
                                        $need_confirm = false, $need_apply = false)
    {
        $apply_action = null;
        if ($need_apply) {
            $apply_action = UserInputHandlerRegistry::create_action($handler, $name, $add_params);
            $apply_action['params']['action_type'] = 'apply';
        }

        $confirm_action = null;
        if ($need_confirm) {
            $confirm_action = UserInputHandlerRegistry::create_action($handler, $name, $add_params);
            $confirm_action['params']['action_type'] = 'confirm';
        }

        $defs[] = array
        (
            GuiControlDef::name => $name,
            GuiControlDef::title => $title,
            GuiControlDef::kind => GUI_CONTROL_COMBOBOX,
            GuiControlDef::specific_def => array
            (
                GuiComboboxDef::initial_value => $initial_value,
                GuiComboboxDef::value_caption_pairs => $value_caption_pairs,
                GuiComboboxDef::width => $width,
                GuiComboboxDef::apply_action => $apply_action,
                GuiComboboxDef::confirm_action => $confirm_action,
            ),
        );
    }
}
