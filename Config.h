/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <variant>
#include <map>
#include "UtilsLib\json_wrapper.h"
#include "uri_stream.h"

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
	enLightIptv,
	enCbilling,
	enOttclub,
	enIptvOnline,
	enVidok,
	enShuraTV,
	enTVClub,
	enFilmax,
	enKineskop,
	enMymagic,
	enRusskoeTV,
	enSmile,
	enLast,
};

constexpr auto APP_SETTINGS = L"Application";

constexpr auto CMP_FLAG_TITLE   = 0x01;
constexpr auto CMP_FLAG_ICON    = 0x02;
constexpr auto CMP_FLAG_ARCHIVE = 0x04;
constexpr auto CMP_FLAG_EPG1    = 0x08;
constexpr auto CMP_FLAG_EPG2    = 0x10;
constexpr auto CMP_FLAG_ALL     = CMP_FLAG_TITLE | CMP_FLAG_ICON | CMP_FLAG_ARCHIVE | CMP_FLAG_EPG1 | CMP_FLAG_EPG2;

constexpr auto REG_WINDOW_POS      = _T("WindowPos");
constexpr auto REG_ICON_WINDOW_POS = _T("IconsWindowPos");
constexpr auto REG_EPG_WINDOW_POS  = _T("EpgWindowPos");
constexpr auto REG_VOD_WINDOW_POS  = _T("VodWindowPos");
constexpr auto REG_ACC_WINDOW_POS  = _T("AccountWindowPos");

// app
constexpr auto REG_NEXT_UPDATE     = _T("NextUpdate");
constexpr auto REG_AVAIL_UPDATE    = _T("AvailableUpdate");

// settings dialog
constexpr auto REG_PLAYER          = _T("Player");
constexpr auto REG_FFPROBE         = _T("FFProbe");
constexpr auto REG_LISTS_PATH      = _T("ListsPath");
constexpr auto REG_OUTPUT_PATH     = _T("PluginsPath");
constexpr auto REG_WEB_UPDATE_PATH = _T("PluginsWebUpdatePath");
constexpr auto REG_AUTO_SYNC       = _T("AutoSyncChannel");
constexpr auto REG_AUTO_HIDE       = _T("AutoHideToTray");
constexpr auto REG_MAX_THREADS     = _T("MaxStreamThreads");
constexpr auto REG_LANGUAGE        = _T("Language");
constexpr auto REG_CMP_FLAGS       = _T("CompareFlags");
constexpr auto REG_UPDATE_FREQ     = _T("UpdateFrequencies");
constexpr auto REG_UPDATE_PL       = _T("UpdatePlaylists");
constexpr auto REG_COLOR_ADDED     = _T("ColorAdded");
constexpr auto REG_COLOR_NOT_ADDED = _T("ColorNotAdded");
constexpr auto REG_COLOR_CHANGED   = _T("ColorChanged");
constexpr auto REG_COLOR_HEVC      = _T("ColorHEVC");

// main dialog
constexpr auto REG_PLUGIN          = _T("PluginType");
constexpr auto REG_ICON_SOURCE     = _T("IconSource");
constexpr auto REG_DAYS_BACK       = _T("DaysBack");
constexpr auto REG_HOURS_BACK      = _T("HoursBack");
constexpr auto REG_SHOW_URL        = _T("ShowStreamUrl");

// Plugin dependent

// accounts data
constexpr auto REG_LOGIN               = _T("Login");
constexpr auto REG_PASSWORD            = _T("Password");
constexpr auto REG_TOKEN               = _T("AccessKey");
constexpr auto REG_DOMAIN              = _T("Domain");
constexpr auto REG_VPORTAL             = _T("VPortal");
constexpr auto REG_EMBED_INFO          = _T("EmbedInfo");
constexpr auto REG_DEVICE_ID           = _T("DeviceID");
constexpr auto REG_PROFILE_ID          = _T("ProfileID");
constexpr auto REG_QUALITY_ID          = _T("QualityID");

constexpr auto REG_FILTER_STRING_S     = _T("FilterString");
constexpr auto REG_FILTER_STRING_H     = _T("FilterStringHide");
constexpr auto REG_FILTER_STRING_LST_S = _T("FilterStringList");
constexpr auto REG_FILTER_STRING_LST_H = _T("FilterStringListHide");
constexpr auto REG_FILTER_REGEX_S      = _T("FilterUseRegex");
constexpr auto REG_FILTER_REGEX_H      = _T("FilterUseRegexHide");
constexpr auto REG_FILTER_CASE_S       = _T("FilterUseCase");
constexpr auto REG_FILTER_CASE_H       = _T("FilterUseCaseHide");
constexpr auto REG_FILTER_STATE_S      = _T("FilterState");
constexpr auto REG_FILTER_STATE_H      = _T("FilterStateHide");
constexpr auto REG_CHANNELS_TYPE       = _T("ChannelsType");
constexpr auto REG_PLAYLIST_TYPE       = _T("PlaylistType");
constexpr auto REG_STREAM_TYPE         = _T("StreamType");
constexpr auto REG_CUSTOM_PLAYLIST     = _T("CustomPlaylist");
constexpr auto REG_CREDENTIALS         = _T("Credentials");
constexpr auto REG_LIST_DOMAIN         = _T("ListDomain");
constexpr auto REG_EPG_DOMAIN          = _T("EpgDomain");
constexpr auto REG_PLUGIN_SUFFIX       = _T("PluginSuffix");
constexpr auto REG_ACCOUNT_DATA        = _T("AccountData");
constexpr auto REG_ACTIVE_ACCOUNT      = _T("ActiveAccount");
constexpr auto REG_ACTIVE_CH_LIST      = _T("ActiveChannelsList");

typedef struct
{
	StreamType type;
	std::wstring title;
	std::string short_name;
	std::string int_name;
} PluginDesc;

class Credentials
{
public:
	Credentials() = default;
	void Clear()
	{
		login.clear();
		password.clear();
		token.clear();
		domain.clear();
		portal.clear();
		comment.clear();
		suffix.clear();
		caption.clear();
		logo.clear();
		background.clear();
		update_url.clear();
		update_package_url.clear();
		update_name.clear();
		package_name.clear();
		version_id.clear();
		ch_web_path.clear();
		ch_list.clear();
		custom_increment = 0;
		custom_update_name = 0;
		custom_package_name = 0;
		device_id = 0;
		profile_id = 0;
		embed = 0;
		not_valid = false;
	}

	std::wstring get_login() const { return utils::utf8_to_utf16(login); }
	std::wstring get_password() const { return utils::utf8_to_utf16(password); }
	std::wstring get_token() const { return utils::utf8_to_utf16(token); }
	std::wstring get_domain() const { return utils::utf8_to_utf16(domain); }
	std::wstring get_portal() const { return utils::utf8_to_utf16(portal); }
	std::wstring get_comment() const { return utils::utf8_to_utf16(comment); }
	std::wstring get_suffix() const { return utils::utf8_to_utf16(suffix); }
	std::wstring get_caption() const { return utils::utf8_to_utf16(caption); }
	std::wstring get_logo() const { return utils::utf8_to_utf16(logo); }
	std::wstring get_background() const { return utils::utf8_to_utf16(background); }
	std::wstring get_ch_web_path() const { return utils::utf8_to_utf16(ch_web_path); }

	void set_login(const std::wstring& value) { login = utils::utf16_to_utf8(value); }
	void set_password(const std::wstring& value) { password = utils::utf16_to_utf8(value); }
	void set_token(const std::wstring& value) { token = utils::utf16_to_utf8(value); }
	void set_domain(const std::wstring& value) { domain = utils::utf16_to_utf8(value); }
	void set_portal(const std::wstring& value) { portal = utils::utf16_to_utf8(value); }
	void set_comment(const std::wstring& value) { comment = utils::utf16_to_utf8(value); }
	void set_suffix(const std::wstring& value) { suffix = utils::utf16_to_utf8(value); }
	void set_caption(const std::wstring& value) { caption = utils::utf16_to_utf8(value); }
	void set_logo(const std::wstring& value) { logo = utils::utf16_to_utf8(value); }
	void set_background(const std::wstring& value) { background = utils::utf16_to_utf8(value); }

public:
	std::string login;
	std::string password;
	std::string token;
	std::string domain;
	std::string portal;
	std::string comment;
	std::string suffix;
	std::string caption;
	std::string logo;
	std::string background;
	std::string update_url;
	std::string update_package_url;
	std::string version_id;
	std::string update_name;
	std::string package_name;
	std::string ch_web_path;
	int custom_increment = 0;
	int custom_update_name = 0;
	int custom_package_name = 0;
	int device_id = 0;
	int profile_id = 0;
	int quality_id = 0;
	int embed = 0;
	std::vector<std::string> ch_list;
	bool not_valid = false;
};

void to_json(nlohmann::json& j, const Credentials& c);
void from_json(const nlohmann::json& j, Credentials& c);

//////////////////////////////////////////////////////////////////////////

class ThreadConfig
{
public:
	void SendNotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	void PostNotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	std::vector<BYTE>* m_data = nullptr;
	void* m_parent = nullptr;
	HANDLE m_hStop = nullptr;
	StreamType m_pluginType = StreamType::enEdem;
	std::wstring m_rootPath;
	std::wstring m_url;
	bool m_use_cache = true;
	int m_parser = 0;
};

class PluginsConfig
{
public:
	static PluginsConfig& Instance()
	{
		static PluginsConfig _instance;
		return _instance;
	}
	using map_variant = std::map<std::wstring, std::variant<int, __int64, std::wstring, std::vector<BYTE>>>;

private:
	PluginsConfig() = default;
	~PluginsConfig() = default;

	PluginsConfig(const PluginsConfig& source) = delete;

public:
	void SaveSettings();
	void LoadSettings();

	void SaveSettingsToJson();
	void SaveSettingsToRegistry();

	void UpdatePluginSettings();

	void RemovePortableSettings();

	const std::vector<PluginDesc>& get_plugins_info() const;
	PluginDesc get_plugin_info() const;

	int get_plugin_idx() const;
	void set_plugin_idx(int val);

	StreamType get_plugin_type() const;
	void set_plugin_type(StreamType val);

	std::wstring GetCurrentPluginName(bool bCamel = false) const;

	BOOL IsPortable() const { return m_bPortable; }
	void SetPortable(BOOL val) { m_bPortable = val; }

public:
	std::wstring get_string(bool isApp, const std::wstring& key, const wchar_t* def = L"") const;
	void set_string(bool isApp, const std::wstring& key, const std::wstring& value);

	int get_int(bool isApp, const std::wstring& key, const int def = 0) const;
	void set_int(bool isApp, const std::wstring& key, const int value);

	__int64 get_int64(bool isApp, const std::wstring& key, const __int64 def = 0) const;
	void set_int64(bool isApp, const std::wstring& key, const __int64 value);

	std::vector<BYTE> get_binary(bool isApp, const std::wstring& key) const;
	bool get_binary(bool isApp, const std::wstring& key, LPBYTE* pbData, size_t& dwSize) const;

	void set_binary(bool isApp, const std::wstring& key, const std::vector<BYTE>& value);
	void set_binary(bool isApp, const std::wstring& key, const BYTE* value, const size_t value_size);

	void delete_setting(bool isApp, const std::wstring& key);

public:
	static std::wstring DEV_PATH;
	static std::wstring PACK_DLL_PATH;

protected:
	void ReadSettingsRegistry(StreamType plugin_type);
	void SaveSectionRegistry(StreamType plugin_type);

	bool ReadSettingsJson(StreamType plugin_type);
	void UpdateSettingsJson(StreamType plugin_type);

private:
	std::map<StreamType, map_variant> m_settings;
	StreamType m_pluginType = StreamType::enEdem;
	nlohmann::json m_config;
	BOOL m_bPortable = FALSE;
};

inline PluginsConfig& GetConfig() { return PluginsConfig::Instance(); }

template<typename T>
static std::basic_string<T> GetPluginName(const StreamType plugin_type, bool bCamel = false)
{
	for (const auto& item : GetConfig().get_plugins_info())
	{
		if (item.type != plugin_type) continue;

		std::basic_string<T> plugin_name(item.short_name.begin(), item.short_name.end());
		if (bCamel)
			plugin_name[0] = std::toupper(plugin_name[0]);

		return plugin_name;
	}

	return std::basic_string<T>();
}
