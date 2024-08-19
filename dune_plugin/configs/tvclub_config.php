<?php
require_once 'lib/default_config.php';

class tvclub_config extends default_config
{
    /**
     * @override
     * @inheritDoc
     */
    public function get_format()
    {
        return $this->plugin->get_parameter(PARAM_STREAM_FORMAT, Plugin_Constants::MPEG);
    }

    /**
     * @inheritDoc
     */
    public function get_servers()
    {
        if (empty($this->servers)) {
            try {
                $token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
                $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/servers?token=$token";
                $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url), true);
                foreach ($content['servers'] as $item) {
                    $this->servers[$item['id']] = $item['name'];
                }
            } catch (Exception $ex) {
                hd_debug_print("Servers not loaded");
                print_backtrace_exception($ex);
            }
        }

        return $this->servers;
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: $force");

        if ($force !== false || empty($this->account_data)) {
            if (!$this->load_settings()) {
                return false;
            }
        }

        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function AddSubscriptionUI(&$defs)
    {
        if ($this->GetAccountInfo(true) === false) {
            hd_debug_print("Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
            Control_Factory::add_label($defs, TR::t('description') . ':', TR::t('warn_msg5'), 20);
            return;
        }

        $info = $this->account_data->account->info;
        $settings = $this->account_data->account->settings;
        Control_Factory::add_label($defs, TR::t('balance'), $info->balance . ' €', -10);
        Control_Factory::add_label($defs, TR::t('login'), $info->login, -10);
        Control_Factory::add_label($defs, TR::t('server'), $settings->server_name, -10);
        Control_Factory::add_label($defs, TR::t('time_zone'), "$settings->tz_name $settings->tz_gmt", -10);

        $packages = $this->account_data->account->services;
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, TR::t('packages'), TR::t('no_packages'), 20);
            return;
        }

        foreach ($packages as $item) {
            Control_Factory::add_label($defs, TR::t('package'), $item->name . ' ' . $item->type .' - '. date('j.m.Y', $item['expire']), -10);
        }

        Control_Factory::add_vgap($defs, 20);
    }

    /**
     * @return bool
     */
    protected function load_settings()
    {
        try {
            if (!$this->ensure_token_loaded()) {
                throw new Exception("Token not loaded");
            }

            $token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
            $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/account?token=$token";
            // provider returns token used to download playlist
            $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url), true);
            if (!isset($content['account']['info']['login'])) {
                throw new Exception("Account status unknown");
            }
            $this->account_data = $content;
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            return false;
        }

        return !empty($this->account_data);
    }

    /**
     * @param string $param
     * @return bool
     */
    protected function save_settings($param)
    {
        if (!$this->ensure_token_loaded()) {
            return false;
        }

        try {
            $token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
            $param_set = $this->plugin->get_parameter($param, '');
            $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/set?token=$token&$param=$param_set";
            HD::http_get_document($url);
            $this->load_settings();
            return true;
        } catch (Exception $ex) {
            hd_debug_print("Settings not saved");
            print_backtrace_exception($ex);
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////

    /**
     * @inheritDoc
     */
    protected function ensure_token_loaded($force = false)
    {
        hd_debug_print("Ensure token loaded: $force");
        $login = $this->get_login();
        $password = $this->get_password();

        if (empty($login) || empty($password)) {
            hd_debug_print("Login or password not set");
            return false;
        }

        $token = md5($login . md5($password));
        $old_token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
        if (!empty($old_token) || $old_token !== $token) {
            $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $token);
        }

        return true;
    }
}
