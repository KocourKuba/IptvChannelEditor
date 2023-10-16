<?php
require_once 'lib/default_config.php';

class vidok_config extends default_config
{
    /**
     * @inheritDoc
     */
    public function get_servers()
    {
        $servers = parent::get_servers();
        if (empty($servers) && $this->load_settings()) {
            $servers = array();
            foreach ($this->account_data['settings']['lists']['servers'] as $item) {
                $servers[$item['id']] = $item['name'];
            }
        }

        return $servers;
    }

    /**
     * @inheritDoc
     */
    public function get_server_id()
    {
        if ($this->load_settings()) {
            $server = $this->account_data['settings']['current']['server']['id'];
        } else {
            $server = parent::get_server_id();
        }

        return $server;
    }

    /**
     * @inheritDoc
     */
    public function get_qualities()
    {
        $quality = parent::get_qualities();
        if (empty($quality) && $this->load_settings()) {
            $quality = array();
            foreach ($this->account_data['settings']['lists']['quality'] as $item) {
                $quality[$item['id']] = $item['name'];
            }
        }

        return $quality;
    }

    /**
     * return bool
     */
    protected function load_settings()
    {
        try {
            if (!$this->ensure_token_loaded()) {
                throw new Exception("Token not loaded");
            }

            if (empty($this->account_data)) {
                $token = $this->parent->get_credentials(Ext_Params::M_S_TOKEN);
                $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/settings?token=$token";
                // provider returns token used to download playlist
                $this->account_data = HD::DownloadJson($url);
            }
        } catch (Exception $ex) {
            hd_debug_print($ex->getMessage());
            return false;
        }

        return !empty($this->account_data);
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: $force");

        try {
            if (!$this->ensure_token_loaded()) {
                throw new Exception("User token not loaded");
            }

            if ($force !== false || empty($this->account_data)) {
                $token = $this->parent->get_credentials(Ext_Params::M_S_TOKEN);
                $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/account?token=$token";
                // provider returns token used to download playlist
                $this->account_data = HD::DownloadJson($url);
                if (!isset($this->account_data['account']['login'])) {
                    throw new Exception("Account info invalid");
                }
            }
        } catch (Exception $ex) {
            hd_debug_print($ex->getMessage());
            return false;
        }
        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function AddSubscriptionUI(&$defs)
    {
        $account_data = $this->GetAccountInfo(true);
        if ($account_data === false) {
            hd_debug_print("Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
            Control_Factory::add_label($defs, TR::t('description'), TR::t('warn_msg5'), 20);
            return;
        }

        Control_Factory::add_label($defs, TR::t('balance'), $account_data['account']['balance'] . ' €', -10);
        Control_Factory::add_label($defs, TR::t('login'), $account_data['account']['login'], -10);
        $packages = $account_data['account']['packages'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, TR::t('packages'), TR::t('no_packages'), 20);
            return;
        }

        foreach ($packages as $item) {
            Control_Factory::add_label($defs, TR::t('package'), $item['name'] .' - '. date('j.m.Y', $item['expire']), -10);
        }

        Control_Factory::add_vgap($defs, 20);
    }

    /**
     * @param string $param
     * @return bool
     */
    protected function save_settings($param)
    {
        try {
            if (!$this->ensure_token_loaded()) {
                throw new Exception("User token not loaded");
            }

            $token = $this->parent->get_credentials(Ext_Params::M_S_TOKEN);
            $param_set = $this->parent->get_parameter($param, '');
            $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/settings_set?token=$token&$param=$param_set";
            $json = HD::DownloadJson($url);
            if (isset($json['error'])) {
                throw new Exception($json['error']['msg']);
            }
            return true;
        } catch (Exception $ex) {
            hd_debug_print($ex->getMessage());
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

        $token = md5(strtolower($login) . md5($password));
        $old_token = $this->parent->get_credentials(Ext_Params::M_S_TOKEN);
        if (!empty($old_token) || $old_token !== $token) {
            $this->parent->set_credentials(Ext_Params::M_S_TOKEN, $token);
        }

        return true;
    }
}
