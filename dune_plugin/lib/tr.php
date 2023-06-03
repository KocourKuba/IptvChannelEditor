<?php

class TR
{
    public static function t()
    {
        $num_args = func_num_args();
        $arg_list = func_get_args();
        if ($num_args > 1) {
            $params = '';
            for ($i = 1; $i < $num_args; $i++) {
                $params .= "<p>$arg_list[$i]</p>";
            }
            $str = "%ext%<key_local>$arg_list[0]$params</key_local>";
        } else if ($num_args === 1) {
            $str = "%tr%$arg_list[0]";
        } else {
            $str = '';
        }
        return $str;
    }

    /**
     * @param $key string
     */
    public static function g($key)
    {
        $num_args = func_num_args();
        $arg_list = func_get_args();
        if ($num_args > 1) {
            $params = '';
            for ($i = 1; $i < $num_args; $i++) {
                $params .= "<p>$arg_list[$i]</p>";
            }
            $str = "%ext%<key_global>$arg_list[0]$params</key_global>";
            hd_print($str);
        } else if ($num_args === 1) {
            $str = "%tr%$arg_list[0]";
        } else {
            $str = '';
        }
        return $str;
    }
}
