#include "pch.h"
#include <iomanip>

#include "Config.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// special case for run under debugger from VS
#ifdef _DEBUG
std::wstring PluginsConfig::DEV_PATH = L"..\\";
std::wstring PluginsConfig::PACK_DLL_PATH = L"dll\\";
#else
std::wstring PluginsConfig::DEV_PATH;
std::wstring PluginsConfig::PACK_DLL_PATH;
#endif // _DEBUG

#define MAKEQWORD(a, b) ((QWORD)(((DWORD)(((QWORD)(a)) & 0xffffffff)) | ((QWORD)((DWORD)(((QWORD)(b)) & 0xffffffff))) << 32))

constexpr auto MAX_REGVAL_SIZE = 1024; // real max size - 32767 bytes;
constexpr auto REGISTRY_APP_ROOT = LR"(SOFTWARE\Dune IPTV Channel Editor)";
constexpr auto REGISTRY_APP_SETTINGS = LR"(SOFTWARE\Dune IPTV Channel Editor\Editor\Settings)";
constexpr auto CONFIG_FILE = L"settings.cfg";

static std::set<std::wstring> all_settings_keys = {
	REG_WINDOW_POS,
	REG_ICON_WINDOW_POS,
	REG_EPG_WINDOW_POS,
	REG_PLAYER,
	REG_FFPROBE,
	REG_LISTS_PATH,
	REG_OUTPUT_PATH,
	REG_AUTO_SYNC,
	REG_AUTO_HIDE,
	REG_MAX_THREADS,
	REG_LANGUAGE,
	REG_CMP_FLAGS,
	REG_UPDATE_FREQ,
	REG_UPDATE_PL,
	REG_PLUGIN,
	REG_ICON_SOURCE,
	REG_DAYS_BACK,
	REG_HOURS_BACK,
	REG_NEXT_UPDATE,
	REG_AVAIL_UPDATE,
	REG_LOGIN,
	REG_PASSWORD,
	REG_TOKEN,
	REG_DOMAIN,
	REG_ACCESS_URL,
	REG_HOST,
	REG_PORTAL,
	REG_FILTER_STRING_S,
	REG_FILTER_STRING_H,
	REG_FILTER_STRING_LST_S,
	REG_FILTER_STRING_LST_H,
	REG_FILTER_REGEX_S,
	REG_FILTER_CASE_S,
	REG_FILTER_REGEX_H,
	REG_FILTER_CASE_H,
	REG_FILTER_STATE_S,
	REG_FILTER_STATE_H,
	REG_CHANNELS_TYPE,
	REG_PLAYLIST_TYPE,
	REG_STREAM_TYPE,
	REG_CUSTOM_PLAYLIST,
	REG_DEVICE_ID,
	REG_SHOW_URL,
	REG_CREDENTIALS,
};

static std::vector<PluginDesc> all_plugins = {
	{ StreamType::enAntifriz,   _T("Antifriz TV"),     "antifriz",   "antifriz.tv",    "1.0.3" },
	{ StreamType::enEdem,       _T("iEdem/iLook TV"),  "edem",       "iedem.tv",       "2.7.3" },
	{ StreamType::enFox,        _T("Fox TV"),          "fox",        "fox-fun.tv",     "1.0.1" },
	{ StreamType::enGlanz,      _T("glanz TV"),        "glanz",      "glanz.tv",       "1.0.3" },
	{ StreamType::enItv,        _T("ITV Live TV"),     "itv",        "itv-live.tv",    "1.0.1" },
	{ StreamType::enSharaclub,  _T("Sharaclub TV"),    "sharaclub",  "sharaclub.tv",   "1.0.2" },
	{ StreamType::enSharavoz,   _T("Sharavoz TV"),     "sharavoz",   "sharavoz.tv",    "1.0.2" },
	{ StreamType::enOneCent,    _T("1CENT TV"),        "onecent",    "onecent.tv",     "1.0.0" },
	{ StreamType::enOneUsd,     _T("1USD TV"),         "oneusd",     "oneusd.tv",      "1.0.1" },
	{ StreamType::enVipLime,    _T("VipLime TV"),      "viplime",    "viplime.fun.tv", "1.0.0" },
	{ StreamType::enSharaTV,    _T("Shara TV"),        "sharatv",    "shara.tv",       "1.0.0" },
	{ StreamType::enTvTeam,     _T("TV Team"),         "tvteam",     "tv.team",        "1.0.0" },
	{ StreamType::enOneOtt,     _T("1OTT TV"),         "oneott",     "oneott.tv",      "1.0.0" },
	{ StreamType::enLightIptv,  _T("LightIPTV"),       "lightiptv",  "lightiptv",      "1.0.0" },
	{ StreamType::enCbilling,   _T("Cbilling TV"),     "cbilling",   "cbillingtv",     "1.0.0" },
	{ StreamType::enOttclub,    _T("OttClub"),         "ottclub",    "ottclub",        "1.0.0" },
	{ StreamType::enIptvOnline, _T("IPTV Online"),     "iptvonline", "iptvonline",     "1.0.0" },
	{ StreamType::enVidok,      _T("Vidok TV"),        "vidok",      "vidok.tv",       "1.0.0" },
	{ StreamType::enShuraTV,    _T("Shura TV"),        "shuratv",    "shura.tv",       "1.0.0" },
	{ StreamType::enTVClub,     _T("TV Club"),         "tvclub",     "tvclub",         "1.0.0" },
};

void ThreadConfig::NotifyParent(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CWnd* parent = (CWnd*)m_parent;
	if (parent->GetSafeHwnd())
		parent->SendMessage(message, wParam, lParam);

}

//////////////////////////////////////////////////////////////////////////

void PluginsConfig::SaveSettings()
{
	if (m_bPortable)
		SaveSettingsToJson();
	else
		SaveSettingsToRegistry();
}

void PluginsConfig::LoadSettings()
{
	m_settings.clear();

	std::ifstream in_file(GetAppPath() + CONFIG_FILE);
	if (in_file.good())
	{
		try
		{
			in_file >> m_config;
		}
		catch (const nlohmann::json::out_of_range& ex)
		{
			// out of range errors may happen if provided sizes are excessive
			TRACE("out of range error: %s\n", ex.what());
		}
		catch (const nlohmann::detail::type_error& ex)
		{
			// type error
			TRACE("type error: %s\n", ex.what());
		}
		catch (...)
		{
			TRACE("unknown exception\n");
		}

		if (!m_config.empty())
		{
			bool res = ReadSettingsJson(StreamType::enBase);
			for (const auto& plugin : all_plugins)
			{
				ReadSettingsJson(plugin.type);
			}

			m_bPortable = TRUE;
		}

	}

	if (!m_bPortable)
	{
		m_settings.clear();
		ReadSettingsRegistry(StreamType::enBase);
		for (const auto& plugin : all_plugins)
		{
			m_pluginType = plugin.type;
			ReadSettingsRegistry(plugin.type);
		}
	}

	int idx = get_plugin_idx();
	m_pluginType = (idx >= 0 && idx < (int)all_plugins.size()) ? all_plugins[idx].type : StreamType::enEdem;
}

void PluginsConfig::SaveSettingsToJson()
{
	UpdateSettingsJson(StreamType::enBase);

	for (const auto& plugin : all_plugins)
	{
		UpdateSettingsJson(plugin.type);
	}

	std::ofstream out_file(GetAppPath() + CONFIG_FILE);
	out_file << m_config << std::endl;
}

void PluginsConfig::SaveSettingsToRegistry()
{
	SaveSectionRegistry(StreamType::enBase);

	for (const auto& plugin : all_plugins)
	{
		SaveSectionRegistry(plugin.type);
	}
}

void PluginsConfig::UpdatePluginSettings()
{
	if (m_bPortable)
		UpdateSettingsJson(m_pluginType);
	else
		SaveSectionRegistry(m_pluginType);
}

void PluginsConfig::RemovePortableSettings()
{
	std::error_code ec;
	std::filesystem::remove(GetAppPath() + CONFIG_FILE, ec);
}

std::wstring PluginsConfig::GetCurrentPluginName(bool bCamel /*= false*/) const
{
	return GetPluginName<wchar_t>(m_pluginType, bCamel);
}

const std::vector<PluginDesc>& PluginsConfig::get_plugins_info() const
{
	return all_plugins;
}

PluginDesc PluginsConfig::get_plugin_info(const StreamType plugin) const
{
	for(const auto& info : all_plugins)
	{
		if (info.type == plugin)
			return info;
	}
	return {};
}

int PluginsConfig::get_plugin_idx() const
{
	return get_int(true, REG_PLUGIN);
}

void PluginsConfig::set_plugin_idx(int val)
{
	set_int(true, REG_PLUGIN, val);
	m_pluginType = val < (int)all_plugins.size() ? all_plugins[val].type : StreamType::enEdem;
}

StreamType PluginsConfig::get_plugin_type() const
{
	return m_pluginType;
}

void PluginsConfig::set_plugin_type(StreamType val)
{
	m_pluginType = val;
}

AccountType PluginsConfig::get_plugin_account_type() const
{
	switch (m_pluginType)
	{
		case StreamType::enEdem: // subdomain/token
			return AccountType::enOtt;
		case StreamType::enAntifriz: // pin
		case StreamType::enItv:
		case StreamType::enOneCent:
		case StreamType::enOneUsd:
		case StreamType::enSharavoz:
		case StreamType::enTvTeam:
		case StreamType::enVipLime:
		case StreamType::enLightIptv:
		case StreamType::enCbilling:
		case StreamType::enOttclub:
		case StreamType::enIptvOnline:
		case StreamType::enShuraTV:
			return AccountType::enPin;
		case StreamType::enFox: // login/password
		case StreamType::enGlanz:
		case StreamType::enSharaclub:
		case StreamType::enSharaTV:
		case StreamType::enOneOtt:
		case StreamType::enVidok:
		case StreamType::enTVClub:
			return AccountType::enLoginPass;
		default:
			break;
	}

	return AccountType::enUnknown;
}

std::wstring PluginsConfig::get_string(bool isApp, const std::wstring& key, const wchar_t* def /*= L""*/) const
{
	const auto& section = isApp ? StreamType::enBase : m_pluginType;

	if (m_settings.find(section) == m_settings.end())
		return def;

	const auto& settings = m_settings.at(section);
	if (const auto& pair = settings.find(key); pair != settings.end())
	{
		const auto var = std::get_if<std::wstring>(&pair->second);
		return var ? *var : def;
	}

	return def;
}

void PluginsConfig::set_string(bool isApp, const std::wstring& key, const std::wstring& value)
{
	auto& settings = isApp ? m_settings[StreamType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

int PluginsConfig::get_int(bool isApp, const std::wstring& key, const int def /*= 0*/) const
{
	const auto& section = isApp ? StreamType::enBase : m_pluginType;
	if (m_settings.find(section) == m_settings.end())
		return def;

	const auto& settings = m_settings.at(section);
	if (const auto& pair = settings.find(key); pair != settings.end())
	{
		const auto var = std::get_if<int>(&pair->second);
		return var ? *var : def;
	}

	return def;
}

void PluginsConfig::set_int(bool isApp, const std::wstring& key, const int value)
{
	auto& settings = isApp ? m_settings[StreamType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

__int64 PluginsConfig::get_int64(bool isApp, const std::wstring& key, const __int64 def /*= 0*/) const
{
	const auto& section = isApp ? StreamType::enBase : m_pluginType;
	if (m_settings.find(section) == m_settings.end())
		return def;

	const auto& settings = m_settings.at(section);
	if (const auto& pair = settings.find(key); pair != settings.end())
	{
		const auto var = std::get_if<__int64>(&pair->second);
		return var ? *var : def;
	}

	return def;
}

void PluginsConfig::set_int64(bool isApp, const std::wstring& key, const __int64 value)
{
	auto& settings = isApp ? m_settings[StreamType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

std::vector<BYTE> PluginsConfig::get_binary(bool isApp, const std::wstring& key) const
{
	const auto& section = isApp ? StreamType::enBase : m_pluginType;
	if (m_settings.find(section) != m_settings.end())
	{
		const auto& settings = m_settings.at(section);
		if (const auto& pair = settings.find(key); pair != settings.end())
		{
			const auto var = std::get_if<std::vector<BYTE>>(&pair->second);
			return var ? *var : std::vector<BYTE>();
		}
	}

	return {};
}

bool PluginsConfig::get_binary(bool isApp, const std::wstring& key, LPBYTE* pbData, size_t& dwSize) const
{
	const auto& section = isApp ? StreamType::enBase : m_pluginType;
	if (m_settings.find(section) == m_settings.end())
		return false;

	const auto& settings = m_settings.at(section);
	if (const auto& pair = settings.find(key); pair != settings.end())
	{
		const auto var = std::get_if<std::vector<BYTE>>(&pair->second);
		if (var)
		{
			memcpy(pbData, var->data(), dwSize);
			return true;
		}
	}

	return false;
}

void PluginsConfig::set_binary(bool isApp, const std::wstring & key, const std::vector<BYTE>& value)
{
	auto& settings = isApp ? m_settings[StreamType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

void PluginsConfig::set_binary(bool isApp, const std::wstring& key, const BYTE* value, const size_t value_size)
{
	auto& settings = isApp ? m_settings[StreamType::enBase] : m_settings[m_pluginType];

	settings[key] = std::move(std::vector<BYTE>(value, value + value_size));
}

void PluginsConfig::ReadSettingsRegistry(StreamType plugin_type)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_READ, &hkHive) != ERROR_SUCCESS)
		return;

	const auto& reg_key = fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetPluginName<wchar_t>(plugin_type, false));
	HKEY hKey = nullptr;
	if (::RegOpenKeyExW(hkHive, reg_key.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		auto& settings = m_settings[plugin_type];
		DWORD dwIdx = 0;
		for (;;)
		{
			DWORD dwType = 0;
			DWORD cbData = MAX_REGVAL_SIZE;
			DWORD dwNameSize = MAX_REGVAL_SIZE / sizeof(wchar_t);

			std::vector<wchar_t> szName(dwNameSize);
			std::vector<BYTE> lpData(cbData);

			if (::RegEnumValueW(hKey, dwIdx++, (LPWSTR)szName.data(), &dwNameSize, nullptr, &dwType, (BYTE*)lpData.data(), &cbData) != ERROR_SUCCESS) break;

			std::wstring name(szName.data(), dwNameSize);
			if (all_settings_keys.find(name) == all_settings_keys.end()) // ignore unknown keys
				continue;

			switch (dwType)
			{
				case REG_DWORD:
					settings[name] = *(int*)lpData.data();
					break;
				case REG_QWORD:
					settings[name] = *(__int64*)lpData.data();
					break;
				case REG_SZ:
					settings[name] = (wchar_t*)lpData.data();
					break;
				case REG_BINARY:
					lpData.resize(cbData);
					settings[name] = lpData;
					break;
				default:
					break;
			}
		}

		::RegCloseKey(hKey);
	}

	::RegCloseKey(hkHive);
}

void PluginsConfig::SaveSectionRegistry(StreamType plugin_type)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_WRITE, &hkHive) != ERROR_SUCCESS)
		return;

	const auto& reg_key = fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetPluginName<wchar_t>(plugin_type, false));
	HKEY hKey = nullptr;
	DWORD dwDesp;

	if (::RegCreateKeyExW(hkHive, reg_key.c_str(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, &dwDesp) == ERROR_SUCCESS)
	{
		auto& settings = m_settings[plugin_type];
		for (const auto& pair : settings)
		{
			DWORD dwSize = 0;
			switch (pair.second.index())
			{
				case 0: // int
				{
					auto pbData = (BYTE*)&std::get<int>(pair.second);
					::RegSetValueExW(hKey, pair.first.c_str(), 0, REG_DWORD, pbData, sizeof(int));
					break;
				}
				case 1: // __int64
				{
					auto pbData = (BYTE*)&std::get<__int64>(pair.second);
					::RegSetValueExW(hKey, pair.first.c_str(), 0, REG_QWORD, pbData, sizeof(__int64));
					break;
				}
				case 2: // std::wstring
				{
					const auto& szData = std::get<std::wstring>(pair.second);
					::RegSetValueExW(hKey, pair.first.c_str(), 0, REG_SZ, (LPBYTE)szData.c_str(), (DWORD)(szData.size() * sizeof(wchar_t)));
					break;
				}
				case 3: // std::vector<BYTE>
				{
					const auto& vData = std::get<std::vector<BYTE>>(pair.second);
					::RegSetValueExW(hKey, pair.first.c_str(), 0, REG_BINARY, vData.data(), (DWORD)vData.size());
					break;
				}
			}
		}

		::RegCloseKey(hKey);
	}

	::RegCloseKey(hkHive);
}

bool PluginsConfig::ReadSettingsJson(StreamType plugin_type)
{
	std::string j_section;
	if (plugin_type == StreamType::enBase)
		j_section = utils::utf16_to_utf8(std::wstring_view(APP_SETTINGS));
	else
		j_section = GetPluginName<char>(plugin_type, false);

	if (!m_config.contains(j_section))
		return false;

	auto& settings = m_settings[plugin_type];
	nlohmann::json node = m_config[j_section];
	for (const auto& item : node.items())
	{
		const auto& name = utils::utf8_to_utf16(item.key());
		if (all_settings_keys.find(name) == all_settings_keys.end()) // ignore unknown keys
			continue;

		switch (item.value().type())
		{
			case nlohmann::detail::value_t::number_integer:
			case nlohmann::detail::value_t::number_unsigned:
				settings[name] = item.value().get<int>();
				break;
			case nlohmann::detail::value_t::object:
				if (item.value().contains("qw"))
				{
					auto qw = item.value()["qw"];
					settings[name] = (__int64)MAKEQWORD(qw["ldw"], qw["hdw"]);
				}
				break;
			case nlohmann::detail::value_t::string:
				settings[name] = utils::utf8_to_utf16(item.value().get<std::string>());
				break;
			case nlohmann::detail::value_t::array:
			case nlohmann::detail::value_t::binary:
				settings[name] = item.value().get<std::vector<BYTE>>();
				break;
			default:
				break;
		}
	}

	return true;
}

void PluginsConfig::UpdateSettingsJson(StreamType plugin_type)
{
	auto& settings = m_settings[plugin_type];
	nlohmann::json node;
	for (const auto& pair : settings)
	{
		const auto& name = utils::utf16_to_utf8(pair.first);
		try
		{
			switch (pair.second.index())
			{
				case 0: // int
				{
					node[name] = std::get<int>(pair.second);
					break;
				}
				case 1: // __int64
				{
					__int64 val = std::get<__int64>(pair.second);
					nlohmann::json qw;
					qw["qw"] = nlohmann::json::object({ { "ldw", LODWORD(val) }, { "hdw", HIDWORD(val) } });
					node[name] = qw;
					break;
				}
				case 2: // std::wstring
				{
					node[name] = utils::utf16_to_utf8(std::get<std::wstring>(pair.second));
					break;
				}
				case 3: // std::vector<BYTE>
				{
					node[name] = std::get<std::vector<BYTE>>(pair.second);
					break;
				}
			}
		}
		catch (const nlohmann::json::out_of_range& ex)
		{
			// out of range errors may happen if provided sizes are excessive
			TRACE("out of range error: %s\n", ex.what());
		}
		catch (const nlohmann::detail::type_error& ex)
		{
			// type error
			TRACE("type error: %s\n", ex.what());
		}
		catch (...)
		{
			TRACE("unknown exception\n");
		}
	}

	if (plugin_type == StreamType::enBase)
		m_config[utils::utf16_to_utf8(std::wstring_view(APP_SETTINGS))] = node;
	else
		m_config[GetPluginName<char>(plugin_type, false)] = node;
}
