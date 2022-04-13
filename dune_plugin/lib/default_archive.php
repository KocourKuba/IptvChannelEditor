<?php

require_once 'archive_cache.php';

class Default_Archive implements Archive
{
    public static function clear_cache()
    {
        Archive_Cache::clear_all();
    }

    public static function clear_cached_archive($id)
    {
        Archive_Cache::clear_archive($id);
    }

    /**
     * @param $id
     * @return Archive|null
     */
    public static function get_cached_archive($id)
    {
        return Archive_Cache::get_archive_by_id($id);
    }

    /**
     * @param string $id
     * @param string $url_prefix
     * @return Archive
     */
    public static function get_archive($id, $url_prefix)
    {
        $archive = Archive_Cache::get_archive_by_id($id);
        if (!is_null($archive)) {
            return $archive;
        }

        $version_url = $url_prefix . '/versions.txt';
        try {
            $doc = HD::http_get_document($version_url);
        } catch (Exception $e) {
            $doc = null;
        }

        $version_by_name = array();
        $total_size = 0;

        if (is_null($doc)) {
            hd_print("Failed to fetch archive versions.txt from $version_url.");
        } else {
            $tok = strtok($doc, "\n");
            while ($tok !== null) {
                $pos = strrpos($tok, ' ');
                if ($pos === false) {
                    hd_print("Invalid line in versions.txt for archive '$id'.");
                    continue;
                }

                $name = trim(substr($tok, 0, $pos));
                $version = trim(substr($tok, $pos + 1));
                $version_by_name[$name] = $version;

                $tok = strtok("\n");
            }

            hd_print("Archive $id: " . count($version_by_name) . " files.");

            $size_url = $url_prefix . '/size.txt';
            try {
                $doc = HD::http_get_document($size_url);
            } catch (Exception $ex) {
                hd_print("Failed to fetch archive size.txt from $size_url.");
                $version_by_name = array();
            }

            $total_size = (int)$doc;
            hd_print("Archive $id: size = $total_size");
        }

        $archive = new Default_Archive($id, $url_prefix, $version_by_name, $total_size);

        Archive_Cache::set_archive($archive);

        return $archive;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @var string
     */
    private $id;

    /**
     * @var string
     */
    private $url_prefix;

    /**
     * @var array
     */
    private $version_by_name;

    /**
     * @var int
     */
    private $total_size;

    /**
     * @param string $id
     * @param string $url_prefix
     * @param array $version_by_name
     * @param int $total_size
     */
    public function __construct($id, $url_prefix, $version_by_name, $total_size)
    {
        $this->id = $id;
        $this->url_prefix = $url_prefix;
        $this->version_by_name = $version_by_name;
        $this->total_size = $total_size;
    }

    /**
     * @return string
     */
    public function get_id()
    {
        return $this->id;
    }

    /**
     * @return array
     */
    public function get_archive_def()
    {
        $urls_with_keys = array();
        foreach ($this->version_by_name as $name => $version) {
            $pos = strrpos($name, ".");
            if ($pos === false) {
                $key = $name . '.' . $version;
            } else {
                $key = substr($name, 0, $pos) . '.' .
                    $version . substr($name, $pos);
            }

            $url = $this->url_prefix . "/" . $name;
            $urls_with_keys[$key] = $url;
        }

        return array
        (
            PluginArchiveDef::id => $this->id,
            PluginArchiveDef::urls_with_keys => $urls_with_keys,
            PluginArchiveDef::all_tgz_url => $this->url_prefix . "/all.tgz",
            PluginArchiveDef::total_size => $this->total_size,
        );
    }

    /**
     * @param string $name
     * @return string
     */
    public function get_archive_url($name)
    {
        if (!isset($this->version_by_name[$name])) {
            return "missing://";
        }
        $version = $this->version_by_name[$name];

        $pos = strrpos($name, ".");
        if ($pos === false) {
            $key = $name . '.' . $version;
        } else {
            $key = substr($name, 0, $pos) . '.' .
                $version . substr($name, $pos);
        }

        return 'plugin_archive://' . $this->id . '/' . $key;
    }
}
