<?php
require_once 'lib/default_config.php';

class oneott_config extends default_config
{
    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account");

        if ($force === false && !empty($plugin_cookies->token)) {
            return true;
        }

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        try {
            $url = sprintf('http://list.1ott.net/PinApi/%s/%s', $login, $password);
            // provider returns token used to download playlist
            $account_data = HD::DownloadJson($url);
            if (isset($account_data['token'])) {
                $plugin_cookies->token = $account_data['token'];
                return $account_data;
            }
        } catch (Exception $ex) {
            hd_print("User token not loaded");
        }

        return false;
    }
}
