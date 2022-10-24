<?php

class Abstract_Epfs_Handler
{
	const EPFS_PATH = '/flashdata/plugins_epfs/';

    protected static $dir_path;
    protected static $base_tmp_path;

	///////////////////////////////////////////////////////////////////////

    public static function initialize($plugin_name)
    {
        self::$base_tmp_path = '/tmp/plugins/' . $plugin_name;
        self::$dir_path = self::EPFS_PATH . $plugin_name;
    }

	private static function do_write_epf_data($path, $data, $log_str)
	{
		$tmp_path = "$path.tmp";

		if (false !== file_put_contents($tmp_path, $data))
		{
			if (!rename($tmp_path, $path))
			{
				hd_print("Failed to tmp rename ($path)");
				unlink($tmp_path);
				return;
			}
		}
		else
			hd_print("Failed to write tmp file: $tmp_path");

		hd_print("Write epf for $log_str (" . strlen($data) . ' bytes)');
	}

	////////////////////////////////////////////////////////////////////////////

	protected static function write_no_internet_epf($epf_id, $folder_view)
	{
		if ($folder_view) {
            self::do_write_epf_data(self::get_no_internet_epf_path($epf_id), json_encode($folder_view), "$epf_id.no_internet");
        }
	}

	protected static function write_dummy_epf($epf_id)
	{
		self::write_epf_data($epf_id, '');
	}

	protected static function read_epf_data($epf_id)
	{
		return file_exists($path = self::get_epf_path($epf_id))? file_get_contents($path) : '';
	}

	protected static function write_epf_data($epf_id, $folder_view)
	{
		if ($folder_view)
			self::do_write_epf_data(self::get_epf_path($epf_id), json_encode($folder_view), $epf_id);
	}

	protected static function get_ilang_path()
	{
		return self::$dir_path . '/ilang';
	}

	protected static function get_epfs_ts_path($id)
	{
		return self::$dir_path . "/${id}_timestamp";
	}

	protected static function read_epfs_ts($id)
	{
		return is_file($path = self::get_epfs_ts_path($id)) ? file_get_contents($path) : '';
	}

	protected static function get_epf_path($epf_id)
	{
		return self::$dir_path . "/$epf_id.json";
	}

	protected static function get_no_internet_epf_path($epf_id)
	{
		return self::$dir_path . "/$epf_id.no_internet.json";
	}

	////////////////////////////////////////////////////////////////////////////

	public static function warmed_up_path()
	{
		return self::$base_tmp_path . '/epfs_warmed_up';
	}

	public static function async_worker_warmed_up_path()
	{
		return self::$base_tmp_path . '/async_worker_warmed_up';
	}

	public static function get_epfs_changed_path()
	{
		return self::$base_tmp_path . '/update_epfs_if_needed_flag';
	}

	public static function write_data_ts()
	{
		return is_null($arr = gettimeofday())? 0 : $arr['sec'] . $arr['usec'];
	}
}
