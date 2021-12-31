#include "pch.h"
#include "Config.h"

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

constexpr auto MAX_REGVAL_SIZE = 1024; // real max size - 32767 bytes;
constexpr auto REGISTRY_APP_SETTINGS = LR"(SOFTWARE\Dune IPTV Channel Editor\Editor\Settings)";

static std::set<std::wstring> all_settings_keys = {
	REG_WINDOW_POS,
	REG_ICON_WINDOW_POS,
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
	REG_LOGIN_EMBEDDED,
	REG_PASSWORD,
	REG_PASSWORD_EMBEDDED,
	REG_TOKEN,
	REG_TOKEN_EMBEDDED,
	REG_DOMAIN,
	REG_DOMAIN_EMBEDDED,
	REG_ACCESS_URL,
	REG_HOST,
	REG_HOST_EMBEDDED,
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
	REG_CUSTOM_URL,
	REG_CUSTOM_FILE,
	REG_DEVICE_ID,
};

static std::vector<std::wstring> plugins_images = {
	L"bg_antifriz.jpg",   L"logo_antifriz.png",
	L"bg_edem.jpg",       L"logo_edem.png",
	L"bg_fox.jpg",        L"logo_fox.png",
	L"bg_glanz.jpg",      L"logo_glanz.png",
	L"bg_itv.jpg",        L"logo_itv.png",
	L"bg_onecent.jpg",    L"logo_onecent.png",
	L"bg_oneusd.jpg",     L"logo_oneusd.png",
	L"bg_sharaclub.jpg",  L"logo_sharaclub.png",
	L"bg_sharatv.jpg",    L"logo_sharatv.png",
	L"bg_sharavoz.jpg",   L"logo_sharavoz.png",
	L"bg_tvteam.jpg",     L"logo_tvteam.png",
	L"bg_viplime.jpg",    L"logo_viplime.png",
	L"bg_oneott.jpg",     L"logo_oneott.png",
	L"bg_lightiptv.jpg",  L"logo_lightiptv.png",
	L"bg_cbilling.jpg",   L"logo_cbilling.png",
	L"bg_ottclub.jpg",    L"logo_ottclub.png",
};

static std::vector<PluginDesc> all_plugins = {
	{ StreamType::enAntifriz,  _T("Antifriz"),        "antifriz"   },
	{ StreamType::enEdem,      _T("Edem (iLook TV)"), "edem"       },
	{ StreamType::enFox,       _T("Fox TV"),          "fox"        },
	{ StreamType::enGlanz,     _T("Glanz TV"),        "glanz"      },
	{ StreamType::enItv,       _T("ITV"),             "itv"        },
	{ StreamType::enSharaclub, _T("Sharaclub TV"),    "sharaclub"  },
	{ StreamType::enSharavoz,  _T("Sharavoz TV"),     "sharavoz"   },
	{ StreamType::enOneCent,   _T("1CENT TV"),        "onecent"    },
	{ StreamType::enOneUsd,    _T("1USD TV"),         "oneusd"     },
	{ StreamType::enVipLime,   _T("VipLime TV"),      "viplime"    },
	{ StreamType::enSharaTV,   _T("Shara TV"),        "sharatv"    },
	{ StreamType::enTvTeam,    _T("TV Team"),         "tvteam"     },
	{ StreamType::enOneOtt,    _T("1OTT TV"),         "oneott"     },
	{ StreamType::enLightIptv, _T("LightIPTV"),       "lightiptv"  },
	{ StreamType::enCbilling,  _T("Cbilling"),        "cbilling"   },
	{ StreamType::enOttclub,   _T("Ottclub"),         "ottclub"    },
};

void ThreadConfig::NotifyParent(UINT message, WPARAM wParam, LPARAM lParam)
{
	CWnd* parent = (CWnd*)m_parent;
	if (parent->GetSafeHwnd())
		parent->SendMessage(message, wParam, lParam);

}

//////////////////////////////////////////////////////////////////////////

void PluginsConfig::ReadAppSettingsRegistry()
{
	m_settings.clear();
	ReadSettingsRegistry(REGISTRY_APP_SETTINGS, m_settings);
	int idx = get_plugin_idx();
	m_pluginType = (idx >= 0 && idx < (int)all_plugins.size()) ? all_plugins[idx].type : StreamType::enEdem;
}

void PluginsConfig::SaveAppSettingsRegistry()
{
	SaveSettingsRegistry(REGISTRY_APP_SETTINGS, m_settings);
}

void PluginsConfig::ReadPluginSettingsRegistry()
{
	m_plugin_settings.clear();
	ReadSettingsRegistry(fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetCurrentPluginName()), m_plugin_settings);
}

void PluginsConfig::SavePluginSettingsRegistry()
{
	SaveSettingsRegistry(fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetCurrentPluginName()), m_plugin_settings);
}

std::wstring PluginsConfig::GetCurrentPluginName(bool bCamel /*= false*/) const
{
	return GetPluginName<wchar_t>(m_pluginType, bCamel);
}

const std::vector<PluginDesc>& PluginsConfig::get_plugins_info() const
{
	return all_plugins;
}

const std::vector<std::wstring>& PluginsConfig::get_plugins_images() const
{
	return plugins_images;
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

std::wstring PluginsConfig::get_string(bool isApp, const std::wstring& key, const wchar_t* def /*= L""*/) const
{
	const auto& sel_settings = isApp ? m_settings : m_plugin_settings;

	const auto& pair = sel_settings.find(key);
	if (pair != sel_settings.end())
	{
		const auto var = std::get_if<std::wstring>(&pair->second);
		return var ? *var : def;
	}

	return def;
}

void PluginsConfig::set_string(bool isApp, const std::wstring& key, const std::wstring& value)
{
	auto& settings = isApp ? m_settings : m_plugin_settings;
	settings[key] = value;
}

void PluginsConfig::set_string(bool isApp, const std::wstring& key, const std::string& value)
{
	auto& settings = isApp ? m_settings : m_plugin_settings;
	settings[key] = utils::utf8_to_utf16(value);
}

int PluginsConfig::get_int(bool isApp, const std::wstring& key, const int def /*= 0*/) const
{
	const auto& sel_settings = isApp ? m_settings : m_plugin_settings;

	const auto& pair = sel_settings.find(key);
	if (pair != sel_settings.end())
	{
		const auto var = std::get_if<int>(&pair->second);
		return var ? *var : def;
	}

	return def;
}

void PluginsConfig::set_int(bool isApp, const std::wstring& key, const int value)
{
	auto& sel_settings = isApp ? m_settings : m_plugin_settings;
	sel_settings[key] = value;
}

__int64 PluginsConfig::get_int64(bool isApp, const std::wstring& key, const __int64 def /*= 0*/) const
{
	const auto& sel_settings = isApp ? m_settings : m_plugin_settings;

	const auto& pair = sel_settings.find(key);
	if (pair != sel_settings.end())
	{
		const auto var = std::get_if<__int64>(&pair->second);
		return var ? *var : def;
	}

	return def;
}

void PluginsConfig::set_int64(bool isApp, const std::wstring& key, const __int64 value)
{
	auto& sel_settings = isApp ? m_settings : m_plugin_settings;
	sel_settings[key] = value;
}

std::vector<BYTE> PluginsConfig::get_binary(bool isApp, const std::wstring& key) const
{
	const auto& sel_settings = isApp ? m_settings : m_plugin_settings;
	const auto& pair = sel_settings.find(key);
	if (pair != sel_settings.end())
	{
		const auto var = std::get_if<std::vector<BYTE>>(&pair->second);
		return var ? *var : std::vector<BYTE>();
	}

	return std::vector<BYTE>();
}

bool PluginsConfig::get_binary(bool isApp, const std::wstring& key, LPBYTE* pbData, size_t& dwSize) const
{
	const auto& sel_settings = isApp ? m_settings : m_plugin_settings;
	const auto& pair = sel_settings.find(key);
	if (pair != sel_settings.end())
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
	auto& sel_settings = isApp ? m_settings : m_plugin_settings;
	sel_settings[key] = value;
}

void PluginsConfig::set_binary(bool isApp, const std::wstring& key, const BYTE* value, const size_t value_size)
{
	auto& sel_settings = isApp ? m_settings : m_plugin_settings;
	sel_settings[key] = std::move(std::vector<BYTE>(value, value + value_size));
}

void PluginsConfig::ReadSettingsRegistry(const std::wstring& section, map_variant& settings)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_READ, &hkHive) != ERROR_SUCCESS)
		return;

	HKEY hKey = nullptr;
	if (::RegOpenKeyExW(hkHive, section.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
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

void PluginsConfig::SaveSettingsRegistry(const std::wstring& section, map_variant& settings)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_WRITE, &hkHive) != ERROR_SUCCESS)
		return;

	HKEY hKey = nullptr;
	DWORD dwDesp;

	if (::RegCreateKeyExW(hkHive, section.c_str(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, &dwDesp) == ERROR_SUCCESS)
	{
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
