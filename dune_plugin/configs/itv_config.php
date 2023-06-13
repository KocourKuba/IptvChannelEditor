<?php
require_once 'lib/default_config.php';

class itv_config extends default_config
{
    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] | string[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print(__METHOD__ . ": Collect information from account: $force");

        // this account has special API to get account info
        $password = $this->get_password($plugin_cookies);
        try {
            if (empty($password)) {
                throw new Exception("Password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $url = sprintf($this->get_feature(Plugin_Constants::PROVIDER_API_URL) . '/data/%s', $password);
                $json = HD::DownloadJson($url);
                if (empty($json['package_info'])) {
                    throw new Exception("Account status unknown");
                }
                $this->account_data = $json;
            }

        } catch (Exception $ex) {
            hd_print($ex->getMessage());
            return false;
        }

        return $this->account_data;
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = $this->GetAccountInfo($plugin_cookies, true);
        if ($account_data === false) {
            hd_print(__METHOD__ . ": Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
            Control_Factory::add_label($defs, TR::t('description') . ':', TR::t('warn_msg5'), -10);
            return;
        }

        $title = 'Пакеты: ';

        Control_Factory::add_label($defs, TR::t('balance'), $account_data['user_info']['cash'] . ' $', -10);
        $packages = $account_data['package_info'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, $title, TR::t('no_packages'), 20);
            return;
        }

        $list = array();
        foreach ($packages as $item) {
            $list[] = $item['name'];
        }

        $emptyTitle = str_repeat(' ', strlen($title));
        $list_collected = array();
        $isFirstLabel = true;
        foreach($list as $item) {
            $list_collected[] = $item;
            $collected = implode(', ', $list_collected);
            if (strlen($collected) < 30) {
                continue;
            }

            Control_Factory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, $collected, -10);

            if ($isFirstLabel) {
                $isFirstLabel = false;
            }

            $list_collected = array();
        }

        if (count($list_collected) !== 0) {
            Control_Factory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, implode(', ', $list_collected));
        }

        Control_Factory::add_vgap($defs, 20);
    }
}
