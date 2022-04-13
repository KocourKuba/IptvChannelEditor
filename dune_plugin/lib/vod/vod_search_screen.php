<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

class Vod_Search_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'vod_search';

    /**
     * @return false|string
     */
    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin);

        $plugin->create_screen($this);
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $plugin_cookies
     * @return array
     */
    private function do_get_control_defs(&$plugin_cookies)
    {
        $pattern = '';
        if (isset($plugin_cookies->vod_search_pattern)) {
            $pattern = $plugin_cookies->vod_search_pattern;
        }

        $defs = array();

        Control_Factory::add_label($defs, null, "Enter part of movie name:");

        Control_Factory::add_text_field($defs, $this, null, 'pattern', null,
            $pattern, false, false, true, false,
            500, true, true);

        return $defs;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        return $this->do_get_control_defs($plugin_cookies);
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Vod search: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if ($user_input->action_type === 'apply') {
            $control_id = $user_input->control_id;

            if ($control_id === 'pattern') {
                $pattern = $user_input->pattern;

                $plugin_cookies->vod_search_pattern = $pattern;

                hd_print("Vod search: applying pattern '$pattern'");

                $defs = $this->do_get_control_defs($plugin_cookies);

                return Action_Factory::reset_controls($defs,
                    Action_Factory::open_folder($this->plugin->vod->get_search_media_url_str($pattern), $pattern));
            }
        } else if ($user_input->action_type === 'confirm') {
            $control_id = $user_input->control_id;

            if ($control_id === 'pattern') {
                $pattern = $user_input->pattern;
                if (preg_match('/^\s*$/', $pattern)) {
                    return Action_Factory::show_error(false, 'Pattern should not be empty');
                }
            }
        }

        return null;
    }
}
