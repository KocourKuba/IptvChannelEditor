<?php
require_once 'lib/default_config.php';

class oneott_config extends default_config
{
    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: $force");

        // this account has special API to get account info
        $login = $this->get_login();
        $password = $this->get_password();

        try {
            if (empty($login) || empty($password)) {
                throw new Exception("Login or password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/PinApi/$login/$password";
                // provider returns token used to download playlist
                $json = HD::DownloadJson($url);
                if (!isset($json['token'])) {
                    throw new Exception("User token not loaded");
                }

                $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $json['token']);
                $this->account_data = $json;
            }
        } catch (Exception $ex) {
            hd_debug_print($ex->getMessage());
            return false;
        }

        return $this->account_data;
    }
}
