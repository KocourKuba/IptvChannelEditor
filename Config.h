#pragma once

enum class StreamType
{
	enBase = -2,
	enChannels = -1,
	enAntifriz,
	enEdem,
	enFox,
	enGlanz,
	enItv,
	enOneCent,
	enOneUsd,
	enSharaclub,
	enSharavoz,
	enVipLime,
	enSharaTV,
	enTvTeam,
};

constexpr auto REG_SETTINGS = _T("Settings");
constexpr auto REG_PLAYER = _T("Player");
constexpr auto REG_FFPROBE = _T("FFProbe");
constexpr auto REG_LISTS_PATH = _T("ListsPath");
constexpr auto REG_PLUGINS_PATH = _T("PluginsPath");
constexpr auto REG_DAYS_BACK = _T("DaysBack");
constexpr auto REG_HOURS_BACK = _T("HoursBack");
constexpr auto REG_AUTO_SYNC = _T("AutoSyncChannel");
constexpr auto REG_AUTO_HIDE = _T("AutoHideToTray");
constexpr auto REG_MAX_THREADS = _T("MaxStreamThreads");
constexpr auto REG_PLUGIN = _T("PluginType");
constexpr auto REG_ICON_SOURCE = _T("IconSource");
constexpr auto REG_LANGUAGE = _T("Language");

// Plugin dependent
constexpr auto REG_LOGIN = _T("Login");
constexpr auto REG_LOGIN_EMBEDDED = _T("LoginEmbedded");
constexpr auto REG_PASSWORD = _T("Password");
constexpr auto REG_PASSWORD_EMBEDDED = _T("PasswordEmbedded");
constexpr auto REG_TOKEN = _T("AccessKey");
constexpr auto REG_TOKEN_EMBEDDED = _T("AccessKeyEmbedded");
constexpr auto REG_DOMAIN = _T("Domain");
constexpr auto REG_DOMAIN_EMBEDDED = _T("DomainEmbedded");
constexpr auto REG_ACCESS_URL = _T("AccessUrl");
constexpr auto REG_HOST = _T("Host");
constexpr auto REG_HOST_EMBEDDED = _T("HostEmbedded");
constexpr auto REG_FILTER_STRING = _T("FilterString");
constexpr auto REG_FILTER_REGEX = _T("FilterUseRegex");
constexpr auto REG_FILTER_CASE = _T("FilterUseCase");
constexpr auto REG_FILTER_NOT_ADDED = _T("FilterNotAdded");
constexpr auto REG_CHANNELS_TYPE = _T("ChannelsType");
constexpr auto REG_PLAYLIST_TYPE = _T("PlaylistType");
constexpr auto REG_STREAM_TYPE = _T("StreamType");
constexpr auto REG_CUSTOM_URL = _T("CustomUrl");
constexpr auto REG_CUSTOM_FILE = _T("CustomPlaylist");

typedef struct
{
	StreamType type;
	CString name;
	std::string int_name;
} PluginDesc;

class PluginsConfig
{
public:
	static const std::vector<PluginDesc>& get_plugins_info();

	template<typename T>
	static std::basic_string<T> GetPluginName(const StreamType plugin_type, bool bCamel = false)
	{
		for (const auto& item : PluginsConfig::get_plugins_info())
		{
			if (item.type != plugin_type) continue;

			std::basic_string<T> plugin_name(item.int_name.begin(), item.int_name.end());
			if (bCamel)
				plugin_name[0] = std::toupper(plugin_name[0]);

			return plugin_name;
		}

		return std::basic_string<T>();
	}
};

std::wstring GetAppPath(LPCWSTR szSubFolder = nullptr);

bool PackPlugin(const StreamType plugin_type,
				const std::wstring& output_path,
				const std::wstring& lists_path,
				bool showMessage);
