<?php
require_once 'lib/default_config.php';

class oneott_config extends default_config
{
    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: " . var_export($force, true));

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
                $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::getInstance()->download_content($url));
                if ($content === false || !isset($content->token)) {
                    throw new Exception("User token not loaded");
                }

                $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $content->token);
                $this->account_data = $content;
            }
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            return false;
        }

        return $this->account_data;
    }
}
