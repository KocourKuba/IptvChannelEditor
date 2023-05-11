<?php
require_once 'lib/default_config.php';

class oneott_config extends default_config
{
    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] | string[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account: $force");

        // this account has special API to get account info
        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        try {
            if (empty($login) || empty($password)) {
                throw new Exception("Login or password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $url = sprintf('http://list.1ott.net/PinApi/%s/%s', $login, $password);
                // provider returns token used to download playlist
                $json = HD::DownloadJson($url);
                if (!isset($json['token'])) {
                    throw new Exception("User token not loaded");
                }

                $plugin_cookies->token = $json['token'];
                $this->account_data = $json;
            }
        } catch (Exception $ex) {
            hd_print($ex->getMessage());
            return false;
        }

        return $this->account_data;
    }
}
