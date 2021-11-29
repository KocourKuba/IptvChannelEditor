#pragma once
#include <variant>

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
	enOneOtt,
};

constexpr auto CMP_FLAG_TITLE   = 0x01;
constexpr auto CMP_FLAG_ICON    = 0x02;
constexpr auto CMP_FLAG_ARCHIVE = 0x04;
constexpr auto CMP_FLAG_EPG1    = 0x08;
constexpr auto CMP_FLAG_EPG2    = 0x10;
constexpr auto CMP_FLAG_ALL     = CMP_FLAG_TITLE | CMP_FLAG_ICON | CMP_FLAG_ARCHIVE | CMP_FLAG_EPG1 | CMP_FLAG_EPG2;

constexpr auto FILTER_FLAG_REGEX     = 0x01;
constexpr auto FILTER_FLAG_CASE      = 0x02;
constexpr auto FILTER_FLAG_NOT_ADDED = 0x04;
constexpr auto FILTER_FLAG_CHANGED   = 0x08;

constexpr auto REG_SETTINGS = _T("Settings");

// settings dialog
constexpr auto REG_PLAYER      = _T("Player");
constexpr auto REG_FFPROBE     = _T("FFProbe");
constexpr auto REG_LISTS_PATH  = _T("ListsPath");
constexpr auto REG_OUTPUT_PATH = _T("PluginsPath");
constexpr auto REG_AUTO_SYNC   = _T("AutoSyncChannel");
constexpr auto REG_AUTO_HIDE   = _T("AutoHideToTray");
constexpr auto REG_MAX_THREADS = _T("MaxStreamThreads");
constexpr auto REG_LANGUAGE    = _T("Language");
constexpr auto REG_CMP_FLAGS   = _T("CompareFlags");

// main dialog
constexpr auto REG_PLUGIN      = _T("PluginType");
constexpr auto REG_ICON_SOURCE = _T("IconSource");
constexpr auto REG_DAYS_BACK   = _T("DaysBack");
constexpr auto REG_HOURS_BACK  = _T("HoursBack");

// Plugin dependent
constexpr auto REG_LOGIN             = _T("Login");
constexpr auto REG_LOGIN_EMBEDDED    = _T("LoginEmbedded");
constexpr auto REG_PASSWORD          = _T("Password");
constexpr auto REG_PASSWORD_EMBEDDED = _T("PasswordEmbedded");
constexpr auto REG_TOKEN             = _T("AccessKey");
constexpr auto REG_TOKEN_EMBEDDED    = _T("AccessKeyEmbedded");
constexpr auto REG_DOMAIN            = _T("Domain");
constexpr auto REG_DOMAIN_EMBEDDED   = _T("DomainEmbedded");
constexpr auto REG_ACCESS_URL        = _T("AccessUrl");
constexpr auto REG_HOST              = _T("Host");
constexpr auto REG_HOST_EMBEDDED     = _T("HostEmbedded");
constexpr auto REG_FILTER_STRING     = _T("FilterString");
constexpr auto REG_FILTER_FLAGS      = _T("FilterFlags");
constexpr auto REG_CHANNELS_TYPE     = _T("ChannelsType");
constexpr auto REG_PLAYLIST_TYPE     = _T("PlaylistType");
constexpr auto REG_STREAM_TYPE       = _T("StreamType");
constexpr auto REG_CUSTOM_URL        = _T("CustomUrl");
constexpr auto REG_CUSTOM_FILE       = _T("CustomPlaylist");

typedef struct
{
	StreamType type;
	CString name;
	std::string int_name;
} PluginDesc;

template<typename T>
static std::basic_string<T> GetPluginName(const StreamType plugin_type, bool bCamel = false)
{
	for (const auto& item : GetConfig().get_plugins_info())
	{
		if (item.type != plugin_type) continue;

		std::basic_string<T> plugin_name(item.int_name.begin(), item.int_name.end());
		if (bCamel)
			plugin_name[0] = std::toupper(plugin_name[0]);

		return plugin_name;
	}

	return std::basic_string<T>();
}

//////////////////////////////////////////////////////////////////////////

class ThreadConfig
{
public:
	void NotifyParent(UINT message, WPARAM wParam, LPARAM lParam);

	std::vector<BYTE>* m_data = nullptr;
	CWnd* m_parent = nullptr;
	HANDLE m_hStop = nullptr;
	StreamType m_pluginType = StreamType::enEdem;
	std::wstring m_rootPath;
};

class PluginsConfig
{
public:
	static PluginsConfig& Instance()
	{
		static PluginsConfig _instance;
		return _instance;
	}
	using map_variant = std::map<std::wstring, std::variant<int, std::wstring, std::vector<BYTE>>>;

private:
	PluginsConfig() = default;
	~PluginsConfig() = default;

	PluginsConfig(const PluginsConfig& source) = delete;

public:
	void ReadAppSettingsRegistry();
	void SaveAppSettingsRegistry();

	void ReadPluginSettingsRegistry();
	void SavePluginSettingsRegistry();

	const std::vector<PluginDesc>& get_plugins_info() const;
	const std::vector<std::wstring>& get_plugins_images() const;

	int get_plugin_idx() const;
	void set_plugin_idx(int val);

	StreamType get_plugin_type() const;
	void set_plugin_type(StreamType val);

	std::wstring GetCurrentPluginName(bool bCamel = false) const;

public:
	std::wstring get_string(bool isApp, const std::wstring& key, const wchar_t* def = L"") const;
	void set_string(bool isApp, const std::wstring& key, const std::wstring& value);
	void set_string(bool isApp, const std::wstring& key, const std::string& value);

	int get_int(bool isApp, const std::wstring& key, const int def = 0) const;
	void set_int(bool isApp, const std::wstring& key, const int value);

	std::vector<BYTE> get_binary(bool isApp, const std::wstring& key) const;
	bool get_binary(bool isApp, const std::wstring& key, LPBYTE* pbData, size_t& dwSize) const;

	void set_binary(bool isApp, const std::wstring& key, const std::vector<BYTE>& value);
	void set_binary(bool isApp, const std::wstring& key, const BYTE* value, const size_t value_size);

public:
	static CStringW DEV_PATH;
	static CStringW PACK_DLL_PATH;

protected:
	void ReadSettingsRegistry(const std::wstring& section, map_variant& settings);
	void SaveSettingsRegistry(const std::wstring& section, map_variant& settings);

private:
	map_variant m_settings;
	map_variant m_plugin_settings;
	StreamType m_pluginType = StreamType::enEdem;
};

inline PluginsConfig& GetConfig() { return PluginsConfig::Instance(); }
