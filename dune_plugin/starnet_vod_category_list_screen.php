<?php
///////////////////////////////////////////////////////////////////////////

require_once 'starnet_vod_category.php';
require_once 'starnet_vod_list_screen.php';

///////////////////////////////////////////////////////////////////////////

class StarnetVodCategoryListScreen extends AbstractPreloadedRegularScreen
{
    const ID = 'vod_category_list';

    public static function get_media_url_str($category_id)
    {
        return MediaURL::encode(
            array
            (
                'screen_id' => self::ID,
                'category_id' => $category_id,
            ));
    }

    ///////////////////////////////////////////////////////////////////////
    private $config;
    private $category_list;
    private $category_index;

    ///////////////////////////////////////////////////////////////////////

    public function __construct()
    {
        parent::__construct(self::ID, $this->get_folder_views());
        $this->config = new EdemPluginConfig();
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(
            GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
        );
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        if (is_null($this->category_index))
            $this->fetch_vod_categories();

        $category_list = $this->category_list;

        if (isset($media_url->category_id)) {
            if (!isset($this->category_index[$media_url->category_id])) {
                hd_print("Error: parent category (id: " .
                    $media_url->category_id . ") not found.");
                throw new Exception('No parent category found');
            }

            $parent_category = $this->category_index[$media_url->category_id];
            $category_list = $parent_category->get_sub_categories();
        }

        $items = array();

        if ($this->config->VOD_FAVORITES_SUPPORTED &&
            !isset($media_url->category_id)) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url => VodFavoritesScreen::get_media_url_str(),
                PluginRegularFolderItem::caption => ViewsConfig::FAV_MOVIES_CATEGORY_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => ViewsConfig::FAV_MOVIES_CATEGORY_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => ViewsConfig::FAV_MOVIES_CATEGORY_ICON_PATH,
                )
            );
        }

        foreach ($category_list as $c) {
            $is_movie_list = is_null($c->get_sub_categories());
            $media_url_str = $is_movie_list ?
                StarnetVodListScreen::get_media_url_str($c->get_id()) :
                self::get_media_url_str($c->get_id());

            $items[] = array
            (
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $c->get_caption(),
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $c->get_icon_path(),
                    ViewItemParams::item_detailed_icon_path => $c->get_icon_path()
                )
            );
        }

        return $items;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    private function fetch_vod_categories()
    {
        $doc = HD::http_get_document($this->config->VOD_CATEGORIES_URL);

        if (is_null($doc))
            throw new Exception('Can not fetch playlist');

        $xml = simplexml_load_string($doc);

        if ($xml === false) {
            hd_print("Error: can not parse XML document.");
//            hd_print("XML-text: $doc.");
            throw new Exception('Illegal XML document');
        }

        if ($xml->getName() !== 'vod_categories') {
            hd_print("Error: unexpected node '" . $xml->getName() . "'. Expected: 'vod_categories'");
            throw new Exception('Invalid XML document');
        }

        $this->category_list = array();
        $this->category_index = array();

        $this->fill_categories($xml->children(), $this->category_list);
    }

    ///////////////////////////////////////////////////////////////////////

    private function fill_categories($xml_categories, &$obj_arr)
    {
        foreach ($xml_categories as $c) {
            $cat =
                new StarnetVodCategory(
                    strval($c->id),
                    strval($c->caption),
                    strval($c->icon_url));

            if (isset($c->vod_categories)) {
                $sub_categories = array();
                $this->fill_categories($c->vod_categories->children(), $sub_categories);
                $cat->set_sub_categories($sub_categories);
            }

            $obj_arr[] = $cat;

            $this->category_index[$cat->get_id()] = $cat;
        }
    }

    ///////////////////////////////////////////////////////////////////////

    private function get_folder_views()
    {
        return ViewsConfig::GET_VOD_CATEGORY_LIST_FOLDER_VIEWS();
    }
}

///////////////////////////////////////////////////////////////////////////
?>
