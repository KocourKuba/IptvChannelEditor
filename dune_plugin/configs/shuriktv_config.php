<?php
require_once 'lib/default_config.php';

class shuriktv_config extends default_config
{
    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: " . var_export($force, true));

        // this account has special API to get account info
        $password = $this->get_password();
        try {
            if (empty($password)) {
                throw new Exception("Password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/customers/expired_packet/$password/";
                $json = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url));
                if ($json === false || !isset($json[0])) {
                    throw new Exception("Account status unknown");
                }
                $this->account_data = $json[0];
            }
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            return false;
        }

        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function AddSubscriptionUI(&$defs)
    {
        hd_debug_print(null, true);
        $account_info = $this->GetAccountInfo(true);
        $defs = array();
        Control_Factory::add_vgap($defs, 20);

        if (!isset($account_info)) {
            hd_debug_print("Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
        } else {
            if (isset($account_info->packet)) {
                Control_Factory::add_label($defs, TR::t('package'), $account_info->packet, -15);
            }
            if (isset($account_info->expired)) {
                Control_Factory::add_label($defs, TR::t('end_date'), gmdate("d.m.Y  H:i", substr($account_info->expired, 0, -3)), -15);
            }
        }

        Control_Factory::add_vgap($defs, 20);

        return Action_Factory::show_dialog(TR::t('subscription'), $defs, true, 1000, null /*$attrs*/);
    }
}
