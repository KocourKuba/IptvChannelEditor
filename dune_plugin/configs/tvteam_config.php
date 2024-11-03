<?php

require_once 'lib/default_config.php';

class tvteam_config extends default_config
{
    const API_COMMAND_REQUEST_TOKEN = '{API_URL}/?userLogin={LOGIN}&userPasswd=';
    const API_COMMAND_ACCOUNT_INFO = '{API_URL}/?apiAction=getUserData,getServersGroups,getUserPackages&sessionId=';
    const API_COMMAND_SET_SERVER = '{API_URL}/?apiAction=updateUserData&groupId={SERVER_ID}&sessionId=';

    const SESSION_FILE = "session_id";

    /**
     * @var array
     */
    protected $session_id = array();

    /**
     * @var array
     */
    protected $packages = array();

    /**
     * @inheritDoc
     */
    public function set_server_id($server)
    {
        if (!$this->ensure_token_loaded()) {
            hd_debug_print("Token not loaded");
            return false;
        }

        $old = $this->get_server_id();
        parent::set_server_id($server);

        $response = $this->execApiCommand(self::API_COMMAND_SET_SERVER, null, true, $this->session_id);
        hd_debug_print("SetServer: " . pretty_json_format($response), true);
        if (isset($response->status) && (int)$response->status === 1) {
            $this->account_data = null;
            $this->servers = array();
            return true;
        }

        parent::set_server_id($old);

        return false;
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print(null, true);
        hd_debug_print("Collect information from account: " . var_export($force, true));

        if (!$this->ensure_token_loaded()) {
            hd_debug_print("Token not loaded");
            return null;
        }

        if (empty($this->account_data) || $force) {
            $this->account_data = $this->execApiCommand(self::API_COMMAND_ACCOUNT_INFO, null, true, $this->session_id);
            hd_debug_print("get provider info response: " . pretty_json_format($this->account_data), true);
            if (isset($this->account_data->data->userData->userToken)) {
                $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $this->account_data->data->userData->userToken);
            }

            if (isset($this->account_data->data->serversGroupsList)) {
                foreach ($this->account_data->data->serversGroupsList as $server) {
                    $this->servers[$server->groupId] = "$server->groupCountry ($server->streamDomainName)";
                }
                parent::set_server_id($this->account_data->data->userData->groupId);
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
            Control_Factory::add_label($defs, TR::t('description'), TR::t('warn_msg5'), 20);
            return;
        }

        $info = $this->account_data->data->userData;
        if (isset($info->userLogin)) {
            Control_Factory::add_label($defs, TR::t('login'), $info->userLogin, -15);
        }

        if (isset($info->userEmail)) {
            Control_Factory::add_label($defs, TR::t('name'), $info->userEmail, -15);
        }

        if (isset($info->userBalance)) {
            Control_Factory::add_label($defs, TR::t('balance'), $info->userBalance . "$", -15);
        }

        if (isset($info->groupId)) {
            $name = isset($this->servers[$info->groupId]) ? $this->servers[$info->groupId] : 'Not set';
            Control_Factory::add_label($defs, TR::t('server'), $name, -15);
        }

        if (isset($info->showPorno)) {
            Control_Factory::add_label($defs, TR::t('disable_adult'), $info->showPorno ? TR::t('no') : TR::t('yes'), -15);
        }

        $this->packages = $this->account_data->data->userPackagesList;
        $packages = '';
        foreach ($this->packages as $package) {
            $packages .= TR::load_string('package') . " " . $package->packageName . PHP_EOL;
            $packages .= TR::load_string('start_date') . " " . $package->fromDate . PHP_EOL;
            $packages .= TR::load_string('end_date') . " " . $package->toDate . PHP_EOL;
            $packages .= TR::load_string('money_need') . " " . $package->salePrice . "$" . PHP_EOL;
        }

        Control_Factory::add_multiline_label($defs, TR::t('packages'), $packages, 10);

        Control_Factory::add_vgap($defs, 20);
    }

    //////////////////////////////////////////////////////////////////////

    /**
     * @inheritDoc
     */
    protected function ensure_token_loaded($force = false)
    {
        hd_debug_print(null, true);
        hd_debug_print("force request provider token: " . var_export($force, true));

        $login = $this->get_login();
        $password = $this->get_password();

        if (empty($login) || empty($password)) {
            hd_debug_print("Login or password not set");
            return false;
        }

        $session_file = get_temp_path(self::SESSION_FILE);
        $expired = true;
        if (file_exists($session_file)) {
            $this->session_id[CURLOPT_CUSTOMREQUEST] = file_get_contents($session_file);
            $expired = time() > filemtime($session_file);
            if ($expired) {
                unlink($session_file);
            }
        }

        if (!$force && isset($this->session_id[CURLOPT_CUSTOMREQUEST]) && !$expired) {
            hd_debug_print("request not required", true);
            return true;
        }

        $error_msg = HD::check_last_error('rq_last_error');
        if (!$force && !empty($error_msg)) {
            $info_msg = str_replace('|', PHP_EOL, TR::load_string('err_auth_no_spam'));
            hd_debug_print($info_msg);
            HD::set_last_error("pl_last_error", "$info_msg\n\n$error_msg");
        } else {
            HD::set_last_error("pl_last_error", null);
            HD::set_last_error("rq_last_error", null);
            $curl_options[CURLOPT_CUSTOMREQUEST] = md5($password);
            $response = $this->execApiCommand(self::API_COMMAND_REQUEST_TOKEN, null, true, $curl_options);
            hd_debug_print("request provider token response: " . pretty_json_format($response), true);
            if ($response->status === 0 || !empty($response->error)) {
                HD::set_last_error("pl_last_error", $response->error);
                HD::set_last_error("rq_last_error", $response->error);
            } else if (isset($response->data->sessionId)) {
                file_put_contents($session_file, $response->data->sessionId);
                touch($session_file, time() + 86400);
                HD::set_last_error("rq_last_error", null);
                $this->session_id[CURLOPT_CUSTOMREQUEST] = $response->data->sessionId;
                return true;
            }
        }

        return false;
    }
}
