#pragma once

// Access info
static constexpr auto ACCESS_TEMPLATE_SHARACLUB = "http://list.playtv.pro/api/dune-api5m.php?subscr={:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE_SHARACLUB = "http://list.playtv.pro/tv_live-m3u8/{:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE_GLANZ = "http://pl.ottglanz.tv/get.php?username={:s}&password={:s}&type=m3u&output=hls";
static constexpr auto PLAYLIST_TEMPLATE_ANTIFRIZ = "https://antifriz.tv/playlist/{:s}.m3u8";

// uri templates
static constexpr auto URI_TEMPLATE_EDEM = "http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";
static constexpr auto URI_TEMPLATE_SHARAVOZ = "http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_SHARACLUB = "http://{SUBDOMAIN}/live/{TOKEN}/{ID}/index.m3u8";
static constexpr auto URI_TEMPLATE_GLANZ = "http://{SUBDOMAIN}/{ID}/index.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto URI_TEMPLATE_ANTIFRIZ = "http://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_ANTIFRIZ_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";

// epg templates
static constexpr auto EPG1_TEMPLATE_EDEM = "http://epg.ott-play.com/edem/epg/%s.json";
static constexpr auto EPG1_TEMPLATE_SHARAVOZ = "http://api.program.spr24.net/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG1_TEMPLATE_SHARACLUB = "http://api.sramtv.com/get/?type=epg&ch={:s}&date=&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG1_TEMPLATE_GLANZ = "http://epg.ott-play.com/ottg/epg/%s.json";
static constexpr auto EPG1_TEMPLATE_ANTIFRIZ = "http://epg.ott-play.com/antifriz/epg/%s.json";

static constexpr auto EPG2_TEMPLATE_EDEM = "http://www.teleguide.info/kanal{:d}_{:4d}{:02d}{:02d}.html";
static constexpr auto EPG2_TEMPLATE_SHARAVOZ = "http://epg.arlekino.tv/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE_SHARACLUB = "http://api.gazoni1.com/get/?type=epg&ch={:s}&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE_GLANZ = "http://epg.ott-play.com/ottg/epg/%s.json";
static constexpr auto EPG2_TEMPLATE_ANTIFRIZ = "http://epg.ott-play.com/antifriz/epg/%s.json";

// Common
constexpr auto REG_SETTINGS = _T("Settings");
constexpr auto REG_PLAYER = _T("Player");
constexpr auto REG_FFPROBE = _T("FFProbe");
constexpr auto REG_DAYS_BACK = _T("DaysBack");
constexpr auto REG_HOURS_BACK = _T("HoursBack");
constexpr auto REG_AUTOSYNC = _T("AutoSyncChannel");
constexpr auto REG_PLUGIN = _T("PluginType");
constexpr auto REG_ICON_SOURCE = _T("IconSource");

// Plugin dependent
constexpr auto REG_LOGIN = _T("Login");
constexpr auto REG_PASSWORD = _T("Password");
constexpr auto REG_ACCESS_KEY = _T("AccessKey");
constexpr auto REG_DOMAIN = _T("Domain");
constexpr auto REG_ACCESS_URL = _T("AccessUrl");
constexpr auto REG_INT_ID = _T("IntId");
constexpr auto REG_HOST = _T("Host");
constexpr auto REG_FILTER_STRING = _T("FilterString");
constexpr auto REG_FILTER_REGEX = _T("FilterUseRegex");
constexpr auto REG_FILTER_CASE = _T("FilterUseCase");
constexpr auto REG_CHANNELS_TYPE = _T("ChannelsType");
constexpr auto REG_PLAYLIST_TYPE = _T("PlaylistType");
constexpr auto REG_STREAM_TYPE = _T("StreamType");
constexpr auto REG_CUSTOM_URL = _T("CustomUrl");
constexpr auto REG_CUSTOM_FILE = _T("CustomPlaylist");

