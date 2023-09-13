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
        return $this->parent->get_parameter(PARAM_STREAM_FORMAT, Plugin_Constants::MPEG);
    }

    /**
     * @inheritDoc
     */
    public function get_servers()
    {
        $servers = parent::get_servers();
        if (empty($servers)) {
            try {
                $token = $this->parent->get_credentials(Ext_Params::M_TOKEN);
                $json = HD::DownloadJson($this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/servers?token=$token");
                $servers = array();
                foreach ($json['servers'] as $item) {
                    $servers[$item['id']] = $item['name'];
                }
                $this->set_servers($servers);
            } catch (Exception $ex) {
                hd_debug_print("Servers not loaded");
            }
        }

        return $servers;
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
     * @param array &$defs
     */
    public function AddSubscriptionUI(&$defs)
    {
        if ($this->GetAccountInfo(true) === false) {
            hd_debug_print("Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
            Control_Factory::add_label($defs, TR::t('description') . ':', TR::t('warn_msg5'), 20);
            return;
        }

        $info = $this->account_data['account']['info'];
        $settings = $this->account_data['account']['settings'];
        Control_Factory::add_label($defs, TR::t('balance'), $info['balance'] . ' €', -10);
        Control_Factory::add_label($defs, TR::t('login'), $info['login'], -10);
        Control_Factory::add_label($defs, TR::t('server'), $settings['server_name'], -10);
        Control_Factory::add_label($defs, TR::t('time_zone'), $settings['tz_name'] . " {$settings['tz_gmt']}", -10);

        $packages = $this->account_data['account']['services'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, TR::t('packages'), TR::t('no_packages'), 20);
            return;
        }

        foreach ($packages as $item) {
            Control_Factory::add_label($defs, TR::t('package'), $item['name'] . ' ' . $item['type'] .' - '. date('j.m.Y', $item['expire']), -10);
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

            $token = $this->parent->get_credentials(Ext_Params::M_TOKEN);
            $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/account?token=$token";
            // provider returns token used to download playlist
            $json = HD::DownloadJson($url);
            if (!isset($json['account']['info']['login'])) {
                throw new Exception("Account status unknown");
            }
            $this->account_data = $json;
        } catch (Exception $ex) {
            hd_debug_print($ex->getMessage());
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
            $token = $this->parent->get_credentials(Ext_Params::M_TOKEN);
            $param_set = $this->parent->get_parameter($param, '');
            $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/set?token=$token&$param=$param_set";
            HD::http_get_document($url);
            $this->load_settings();
            return true;
        } catch (Exception $ex) {
            hd_debug_print("Settings not saved");
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////

    /**
     * @return bool
     */
    protected function ensure_token_loaded()
    {
        $login = $this->get_login();
        $password = $this->get_password();

        if (empty($login) || empty($password)) {
            hd_debug_print("Login or password not set");
            return false;
        }

        $token = md5($login . md5($password));
        $old_token = $this->parent->get_credentials(Ext_Params::M_TOKEN);
        if (!empty($old_token) || $old_token !== $token) {
            $this->parent->set_credentials(Ext_Params::M_TOKEN, $token);
        }

        return true;
    }
}
