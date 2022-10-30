<?php
require_once 'lib/abstract_regular_screen.php';
require_once 'lib/user_input_handler_registry.php';
require_once 'lib/smb_tree.php';
require_once 'lib/hd.php';

class Starnet_Folder_Screen extends Abstract_Regular_Screen implements User_Input_Handler
{
    const ID = 'file_list';

    private $counter = 0;

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->GET_FOLDER_VIEWS());

        $plugin->create_screen($this);
    }

    /**
     * @param string $id
     * @param string $caption
     * @param string $filepath
     * @param string $type
     * @param string $ip_path
     * @param string $user
     * @param string $password
     * @param string $nfs_protocol
     * @param string $err
     * @param array $save_data
     * @param string $save_file
     * @param int|null $windowCounter
     * @return false|string
     */
    public static function get_media_url_str($id, $caption, $filepath, $type, $ip_path, $user, $password, $nfs_protocol, $err, $save_data, $save_file, $windowCounter = null)
    {
        //hd_print("StarnetFolderScreen: get_media_url_str: $id");
        //hd_print("StarnetFolderScreen: get_media_url_str: $caption");
        //hd_print("StarnetFolderScreen: get_media_url_str: $filepath");

        return MediaURL::encode
        (
            array
            (
                'screen_id' => $id,
                'caption' => $caption,
                'filepath' => $filepath,
                'type' => $type,
                'ip_path' => $ip_path,
                'user' => $user,
                'password' => $password,
                'nfs_protocol' => $nfs_protocol,
                'err' => $err,
                'save_data' => $save_data,
                'save_file' => $save_file,
                'windowCounter' => $windowCounter
            )
        );
    }

    /**
     * @param $plugin_cookies
     * @param string $dir
     * @return array
     */
    public function get_file_list(&$plugin_cookies, $dir)
    {
        //hd_print("StarnetFolderScreen: get_file_list: $dir");
        $smb_shares = new smb_tree();
        $fileData['folder'] = array();
        $fileData['file'] = array();
        if ($dir === '/tmp/mnt/smb') {
            $info = isset($plugin_cookies->smb_setup) ? (int)$plugin_cookies->smb_setup : 1;
            $s['smb'] = $smb_shares->get_mount_all_smb($info);
            return $s;
        }

        if ($dir === '/tmp/mnt/network') {
            $s['nfs'] = $smb_shares::get_mount_nfs();
            return $s;
        }

        if ($handle = opendir($dir)) {
            $bug_kind = $smb_shares::get_bug_platform_kind();
            while (false !== ($file = readdir($handle))) {
                if ($file !== "." && $file !== "..") {
                    $absolute_filepath = $dir . '/' . $file;
                    $is_match = preg_match('|^/tmp/mnt/smb/|', $absolute_filepath);
                    $is_dir = $bug_kind && $is_match ? (bool)trim(shell_exec("test -d \"$absolute_filepath\" && echo 1 || echo 0")) : is_dir($absolute_filepath);

                    if ($is_dir === false) {
                        $fileData['file'][$file]['size'] = ($bug_kind && $is_match) ? '' : filesize($absolute_filepath);
                        $fileData['file'][$file]['filepath'] = $absolute_filepath;
                    } else if ($absolute_filepath !== '/tmp/mnt/nfs' && $absolute_filepath !== '/tmp/mnt/D') {
                        $fileData['folder'][$file]['filepath'] = $absolute_filepath;
                    }
                }
            }
            closedir($handle);
        }

        return $fileData;
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_print("StarnetFolderScreen: get_action_map: " . $media_url->get_raw_string());
        $actions = array();
        $fs_action = User_Input_Handler_Registry::create_action($this, 'fs_action');
        $save_folder = User_Input_Handler_Registry::create_action($this, 'select_folder');
        $save_folder['caption'] = "Выбрать папку";
        $open_folder = User_Input_Handler_Registry::create_action($this, 'open_folder');
        $open_folder['caption'] = "Открыть папку";
        $create_folder = User_Input_Handler_Registry::create_action($this, 'create_folder');
        $create_folder['caption'] = "Создать папку";
        $reset_folder = User_Input_Handler_Registry::create_action($this, 'reset_folder');
        $reset_folder['caption'] = "Сбросить по умолчанию";
        $smb_setup = User_Input_Handler_Registry::create_action($this, 'smb_setup');
        $smb_setup['caption'] = "Настройки SMB";

        $actions[GUI_EVENT_KEY_ENTER] = $fs_action;
        $actions[GUI_EVENT_KEY_PLAY] = $fs_action;
        $actions[GUI_EVENT_KEY_D_BLUE] = $reset_folder;

        if (is_newer_versions() !== false) {
            $actions[GUI_EVENT_KEY_SETUP] = Action_Factory::replace_path($media_url->windowCounter);
        }

        if (empty($media_url->filepath)) {
            $actions[GUI_EVENT_KEY_B_GREEN] = $smb_setup;
        } else if ($media_url->filepath !== '/tmp/mnt/storage' &&
            $media_url->filepath !== '/tmp/mnt/network' &&
            $media_url->filepath !== '/tmp/mnt/smb') {

            if (!empty($media_url->save_data)) {
                $actions[GUI_EVENT_KEY_A_RED] = $open_folder;
                $actions[GUI_EVENT_KEY_C_YELLOW] = $create_folder;
                $actions[GUI_EVENT_KEY_D_BLUE] = $save_folder;
                $actions[GUI_EVENT_KEY_RIGHT] = $save_folder;
            }

            $actions[GUI_EVENT_TIMER] = User_Input_Handler_Registry::create_action($this, 'timer');
        }
        //hd_print("actions: " . json_encode($actions));
        return $actions;
    }

    /**
     * @param MediaURL $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return array
     */
    public function get_folder_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        //hd_print("StarnetFolderScreen: get_folder_range: $from_ndx, " . $media_url->get_raw_string());
        $items = array();
        $dir = empty($media_url->filepath) ? "/tmp/mnt" : $media_url->filepath;
        $windowCounter = isset($media_url->windowCounter) ? $media_url->windowCounter + 1 : 2;
        $err = false;
        $ip_path = isset($media_url->ip_path) ? $media_url->ip_path : false;
        $nfs_protocol = isset($media_url->nfs_protocol) ? $media_url->nfs_protocol : false;
        $user = isset($media_url->user) ? $media_url->user : false;
        $password = isset($media_url->password) ? $media_url->password : false;
        $save_data = isset($media_url->save_data) ? $media_url->save_data : false;
        $save_file = isset($media_url->save_file) ? $media_url->save_file : false;

        foreach ($this->get_file_list($plugin_cookies, $dir) as $key => $itm) {
            ksort($itm);
            foreach ($itm as $k => $v) {
                if ($key === 'smb') {
                    $caption = $v['foldername'];
                    $filepath = $k;
                    $icon_file = self::get_folder_icon('smb_folder', $filepath);
                    $info = "||SMB folder:|$caption||" . $v['ip'];
                    $type = 'folder';
                    $ip_path = $v['ip'];
                    if (isset($v['user']) && !empty($v['user'])) {
                        $user = $v['user'];
                    }
                    if (isset($v['password']) && !empty($v['password'])) {
                        $password = $v['password'];
                    }
                    if (isset($v['err'])) {
                        $info = "||SMB folder:|ERROR!!!||" . $v['err'];
                        $err = $v['err'];
                    }
                } else if ($key === 'nfs') {
                    $caption = $v['foldername'];
                    $filepath = $k;
                    $icon_file = self::get_folder_icon('smb_folder', $filepath);
                    $info = "||NFS folder:|$caption||" . $v['ip'];
                    $type = 'folder';
                    $ip_path = $v['ip'];
                    $nfs_protocol = $v['protocol'];
                    if (isset($v['err'])) {
                        $info = "||NFS folder:|ERROR!!!||" . $v['err'];
                        $err = $v['err'];
                    }
                } else if ($key === 'folder') {
                    if ($k === 'network') {
                        $caption = 'NFS';
                    } else if ($k === 'smb') {
                        $caption = 'SMB';
                    } else if ($k === 'storage') {
                        $caption = 'Storage';
                    } else {
                        $caption = $k;
                    }

                    $filepath = $v['filepath'];
                    $icon_file = self::get_folder_icon($caption, $filepath);

                    $info = "||Folder:|$caption";
                    if ($filepath !== '/tmp/mnt/storage' &&
                        $filepath !== '/tmp/mnt/network' &&
                        $filepath !== '/tmp/mnt/smb' &&
                        $media_url->filepath !== '/tmp/mnt/storage' &&
                        $media_url->filepath !== '/tmp/mnt/network' &&
                        $media_url->filepath !== '/tmp/mnt/smb' &&
                        $media_url->save_data === 'ch_list_path') {
                        $info .= "||Нажмите синюю D или Вправо для выбора|$caption";
                    }
                    $type = 'folder';
                } else if ($key === 'file') {
                    if (!isset($media_url->save_file->extension)) {
                        continue;
                    }

                    $caption = $k;
                    $icon_file = self::get_file_icon($caption);
                    $size = HD::get_filesize_str($v['size']);
                    $filepath = $v['filepath'];
                    $info = "||File:|$caption||$size";
                    $type = 'file';
                } else {
                    continue;
                }

                $detailed_icon = str_replace('small_icons', 'large_icons', $icon_file);
                $items[] = array
                (
                    PluginRegularFolderItem::caption => $caption,
                    PluginRegularFolderItem::media_url => self::get_media_url_str(
                        'file_list',
                        $caption,
                        $filepath,
                        $type,
                        $ip_path,
                        $user,
                        $password,
                        $nfs_protocol,
                        $err,
                        $save_data,
                        $save_file,
                        $windowCounter
                    ),
                    PluginRegularFolderItem::view_item_params => array(
                        ViewItemParams::icon_path => $icon_file,
                        ViewItemParams::item_detailed_info => $info,
                        ViewItemParams::item_detailed_icon_path => $detailed_icon
                    )
                );
            }
        }

        if (empty($items)) {
            $info = "||Нажмите синюю D или Вправо для выбора|$media_url->caption||Файлы показываются";

            $items[] = array(
                PluginRegularFolderItem::caption => '',
                PluginRegularFolderItem::media_url => '',
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => 'gui_skin://small_icons/info.aai',
                    ViewItemParams::item_detailed_icon_path => 'gui_skin://large_icons/info.aai',
                    ViewItemParams::item_detailed_info => $info,
                )
            );
        }

        //hd_print("folder items: " . count($items));
        return array(
            PluginRegularFolderRange::total => count($items),
            PluginRegularFolderRange::more_items_available => false,
            PluginRegularFolderRange::from_ndx => 0,
            PluginRegularFolderRange::count => count($items),
            PluginRegularFolderRange::items => $items
        );
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('smart_file_screen: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $attrs['dialog_params'] = array('frame_style' => DIALOG_FRAME_STYLE_GLASS);
        $selected_url = MediaURL::decode($user_input->selected_media_url);
        $parent_url = MediaURL::decode($user_input->parent_media_url);

        switch ($user_input->control_id) {
            case 'timer':
                $actions = $this->get_action_map($parent_url, $plugin_cookies);
                if (isset($parent_url->filepath)
                    && $parent_url->filepath !== '/tmp/mnt/smb'
                    && $parent_url->filepath !== '/tmp/mnt/network') {
                    $invalidate = Action_Factory::invalidate_folders(array($user_input->parent_media_url));
                } else {
                    $invalidate = null;
                }

                return Action_Factory::change_behaviour($actions, 1000, $invalidate);

            case 'fs_action':
                if ($selected_url->type !== 'folder') {
                    break;
                }

                $caption = $selected_url->caption;
                if ($selected_url->err === false) {
                    return Action_Factory::open_folder($user_input->selected_media_url, $caption);
                }

                $defs = array();
                if ($selected_url->nfs_protocol !== false) {
                    Control_Factory::add_multiline_label($defs, 'Error mount:', $selected_url->err, 3);
                    Control_Factory::add_label($defs, 'NFS folder:', $selected_url->caption);
                    Control_Factory::add_label($defs, 'NFS IP:', $selected_url->ip_path);
                    Control_Factory::add_label($defs, 'Transport Protocol:', $selected_url->nfs_protocol);
                    Control_Factory::add_close_dialog_button($defs, 'ОК', 300);
                    return Action_Factory::show_dialog('Error NFS!!!', $defs, true);
                }

                Control_Factory::add_multiline_label($defs, 'Error mount:', $selected_url->err, 4);
                Control_Factory::add_label($defs, 'SMB folder:', $selected_url->caption);
                Control_Factory::add_label($defs, 'SMB IP:', $selected_url->ip_path);
                if (strpos("Permission denied", $selected_url->err) !== false) {
                    $user = isset($selected_url->user) ? $selected_url->user : '';
                    $password = isset($selected_url->password) ? $selected_url->password : '';
                    $this->GetSMBAccessDefs($defs, $user, $password);
                } else {
                    Control_Factory::add_label($defs, '', '');
                    Control_Factory::add_close_dialog_button($defs, 'ОК', 300);
                }
                return Action_Factory::show_dialog('Error SMB!!!', $defs, true, 1100);

            case 'select_folder':
                $url = isset($selected_url->filepath) ? $selected_url : $parent_url;
                smb_tree::set_folder_info($plugin_cookies, $url);
                $setup_handler = User_Input_Handler_Registry::get_instance()->get_registered_handler(Starnet_Setup_Screen::ID . '_handler');
                $action = is_null($setup_handler) ? null : User_Input_Handler_Registry::create_action($setup_handler, 'reset_controls');
                $post_action = Action_Factory::invalidate_folders(array('setup', 'main_menu', 'main_screen'), $action);

                if (is_newer_versions() !== false) {
                    $post_action = Action_Factory::replace_path($parent_url->windowCounter, null, $post_action);
                }

                return Action_Factory::show_title_dialog('Выбрана папка: ' . $url->caption, $post_action, 'Полный путь: ' . $url->filepath, 800);

            case 'reset_folder':
                $plugin_cookies->ch_list_path = '';
                $setup_handler = User_Input_Handler_Registry::get_instance()->get_registered_handler(Starnet_Setup_Screen::ID . "_handler");
                $action = is_null($setup_handler) ? null : User_Input_Handler_Registry::create_action($setup_handler, 'reset_controls');
                $post_action = Action_Factory::replace_path($parent_url->windowCounter, null,
                    Action_Factory::invalidate_folders(array('setup', 'main_menu', 'main_screen'), $action));

                return Action_Factory::show_title_dialog('Выбрана папка по умолчанию ', $post_action, 'Полный путь: ' . get_install_path(), 800);

            case 'create_folder':
                $defs = array();
                Control_Factory::add_text_field($defs,
                    $this, null,
                    'do_folder_name', '',
                    '', 0, 0, 1, 1, 1230, false, true
                );
                Control_Factory::add_vgap($defs, 500);
                return Action_Factory::show_dialog('Задайте имя папки', $defs, true);

            case 'do_folder_name':
                $do_mkdir = User_Input_Handler_Registry::create_action($this, 'do_mkdir');
                return Action_Factory::close_dialog_and_run($do_mkdir);

            case 'do_mkdir':
                if (!mkdir($concurrentDirectory = $parent_url->filepath . '/' . $user_input->do_folder_name) && !is_dir($concurrentDirectory)) {
                    return Action_Factory::show_title_dialog('Невозможно создать папку!');
                }
                return Action_Factory::invalidate_folders(array($user_input->parent_media_url));

            case 'open_folder':
                $path = $parent_url->filepath;
                if (preg_match('|^/tmp/mnt/storage/|', $path)) {
                    $path = preg_replace('|^/tmp/mnt/storage/|', 'storage_name://', $path);
                } else if (isset($parent_url->ip_path)) {
                    if (preg_match('|^/tmp/mnt/smb/|', $path)) {
                        if ($parent_url->user !== false && $parent_url->password !== false) {
                            $path = 'smb://' . $parent_url->user . ':' . $parent_url->password . '@' . preg_replace("|^/tmp/mnt/smb/\d|", str_replace('//', '', $parent_url->ip_path), $path);
                        } else {
                            $path = 'smb:' . preg_replace("|^/tmp/mnt/smb/\d|", $parent_url->ip_path, $path);
                        }
                    } else if ($parent_url->nfs_protocol !== false && preg_match('|^/tmp/mnt/network/|', $path)) {
                        $prot = ($parent_url->nfs_protocol === 'tcp') ? 'nfs-tcp://' : 'nfs-udp://';
                        $path = $prot . preg_replace("|^/tmp/mnt/network/\d|", $parent_url->ip_path . ':/', $path);
                    }
                }
                $url = 'embedded_app://{name=file_browser}{url=' . $path . '}{caption=File Browser}';
                return Action_Factory::launch_media_url($url);

            case 'new_smb_data':
                $smb_shares = new smb_tree ();
                $new_ip_smb[$selected_url->ip_path]['foldername'] = $selected_url->caption;
                $new_ip_smb[$selected_url->ip_path]['user'] = $user_input->new_user;
                $new_ip_smb[$selected_url->ip_path]['password'] = $user_input->new_pass;
                $q = $smb_shares::get_mount_smb($new_ip_smb);
                $key = 'err_' . $selected_url->caption;
                if (isset($q[$key])) {
                    $defs = $this->do_get_mount_smb_err_defs($q[$key]['err'],
                        $selected_url->caption,
                        $selected_url->ip_path,
                        $user_input->new_user,
                        $user_input->new_pass);
                    return Action_Factory::show_dialog('Error SMB!!!', $defs, true, 1100);
                }

                $caption = $selected_url->caption;
                $selected_url = self::get_media_url_str(
                    'file_list',
                    $selected_url->caption,
                    key($q),
                    $selected_url->type,
                    $selected_url->ip_path,
                    $user_input->new_user,
                    $user_input->new_pass,
                    false,
                    false,
                    $selected_url->save_data,
                    $selected_url->save_file
                );
                return Action_Factory::open_folder($selected_url, $caption);

            case 'smb_setup':
                $smb_view = isset($plugin_cookies->smb_setup) ? (int)$plugin_cookies->smb_setup : 1;

                $smb_view_ops[1] = 'Сетевые папки';
                $smb_view_ops[2] = 'Сетевые папки + поиск SMB шар';
                $smb_view_ops[3] = 'Поиск SMB шар';

                $defs = array();
                Control_Factory::add_combobox($defs, $this, null,
                    'smb_view', 'Отображать:',
                    $smb_view, $smb_view_ops, 0
                );
                $save_smb_setup = User_Input_Handler_Registry::create_action($this, 'save_smb_setup');
                Control_Factory::add_custom_close_dialog_and_apply_buffon($defs,
                    '_do_save_smb_setup', 'Применить', 250, $save_smb_setup
                );

                return Action_Factory::show_dialog('Настройка поиска SMB', $defs, true, 1000, $attrs);

            case 'save_smb_setup':
                $smb_view_ops = array();
                $smb_view = 1;
                $smb_view_ops[1] = 'Сетевые папки';
                $smb_view_ops[2] = 'Сетевые папки + поиск SMB папок';
                $smb_view_ops[3] = 'Поиск SMB папок';
                if (isset($user_input->smb_view)) {
                    $smb_view = $user_input->smb_view;
                    $plugin_cookies->smb_setup = $user_input->smb_view;
                }

                return Action_Factory::show_title_dialog("Используется: " . $smb_view_ops[$smb_view]);
        }
        return null;
    }

    /**
     * @param string $ref
     * @return string
     */
    public static function get_file_icon($ref)
    {
        $file_icon = 'gui_skin://small_icons/unknown_file.aai';
        $audio_pattern = '|\.(mp3|ac3|wma|ogg|ogm|m4a|aif|iff|mid|mpa|ra|wav|flac|ape|vorbis|aac|a52)$|i';
        $video_pattern = '|\.(avi|mp4|mpg|mpeg|divx|m4v|3gp|asf|wmv|mkv|mov|ogv|vob|flv|ts|3g2|swf|asf|ps|qt|m2ts)$|i';
        $image_pattern = '|\.(png|jpg|jpeg|bmp|gif|psd|pspimage|thm|tif|yuf|svg|aai|ico|djpg|dbmp|dpng|image_file.aai)$|i';
        $play_list_pattern = '|\.(m3u|m3u8|pls|xml)$|i';
        $torrent_pattern = '|\.torrent$|i';
        if (preg_match($audio_pattern, $ref)) {
            $file_icon = 'gui_skin://small_icons/audio_file.aai';
        }
        if (preg_match($video_pattern, $ref)) {
            $file_icon = 'gui_skin://small_icons/video_file.aai';
        }
        if (preg_match($image_pattern, $ref)) {
            $file_icon = 'gui_skin://small_icons/image_file.aai';
        }
        if (preg_match($play_list_pattern, $ref)) {
            $file_icon = 'gui_skin://small_icons/playlist_file.aai';
        }
        if (preg_match($torrent_pattern, $ref)) {
            $file_icon = 'gui_skin://small_icons/torrent_file.aai';
        }
        return $file_icon;
    }

    /**
     * @param string $caption
     * @param string $filepath
     * @return string
     */
    public static function get_folder_icon($caption, $filepath)
    {
        if ($caption === 'Storage') {
            $folder_icon = "gui_skin://small_icons/sources.aai";
        } else if ($caption === 'SMB') {
            $folder_icon = "gui_skin://small_icons/smb.aai";
        } else if ($caption === 'smb_folder') {
            $folder_icon = "gui_skin://small_icons/network_folder.aai";
        } else if ($caption === 'NFS') {
            $folder_icon = "gui_skin://small_icons/network.aai";
        } else if (preg_match("|/tmp/mnt/storage/.*$|", $filepath) && !preg_match("|/tmp/mnt/storage/.*/|", $filepath)) {
            $folder_icon = "gui_skin://small_icons/hdd.aai";
        } else {
            $folder_icon = "gui_skin://small_icons/folder.aai";
        }

        return $folder_icon;
    }

    /**
     * @param string $err
     * @param string $caption
     * @param string $ip_path
     * @param string $user
     * @param string $password
     * @return array
     */
    public function do_get_mount_smb_err_defs($err, $caption, $ip_path, $user, $password)
    {
        //hd_print('StarnetFolderScreen: do_get_mount_smb_err_defs');
        $defs = array();
        Control_Factory::add_multiline_label($defs, 'Error mount:', $err, 4);
        Control_Factory::add_label($defs, 'SMB folder:', $caption);
        Control_Factory::add_label($defs, 'SMB IP:', $ip_path);
        if (strpos("Permission denied", $err) !== false) {
            $this->GetSMBAccessDefs($defs, $user, $password);
        } else {
            Control_Factory::add_label($defs, '', '');
            Control_Factory::add_close_dialog_button($defs, 'ОК', 300);
        }
        return $defs;
    }

    /**
     * @param array &$defs
     * @param string $user
     * @param string $password
     */
    protected function GetSMBAccessDefs(array &$defs, $user, $password)
    {
        //hd_print('StarnetFolderScreen: GetSMBAccessDefs');
        Control_Factory::add_text_field($defs, $this, null,
            'new_user',
            'Имя пользователя SMB папки:',
            $user, 0, 0, 0, 1, 500
        );

        Control_Factory::add_text_field($defs, $this, null,
            'new_pass',
            'Пароль SMB папки:',
            $password, 0, 0, 0, 1, 500
        );

        Control_Factory::add_custom_close_dialog_and_apply_buffon($defs,
            'new_smb_data', 'Применить', 300,
            User_Input_Handler_Registry::create_action($this, 'new_smb_data')
        );

        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
    }
}
