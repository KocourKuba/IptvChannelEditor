#include "pch.h"
#include "AccountSettings.h"
#include "Constants.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
// special case for run under debugger from VS
std::wstring AccountSettings::DEV_PATH = L"..\\";
std::wstring AccountSettings::PACK_DLL_PATH = L"dll\\";
#else
std::wstring AccountSettings::DEV_PATH;
std::wstring AccountSettings::PACK_DLL_PATH;
#endif // _DEBUG

#define MAKEQWORD(a, b) ((QWORD)(((DWORD)(((QWORD)(a)) & 0xffffffff)) | ((QWORD)((DWORD)(((QWORD)(b)) & 0xffffffff))) << 32))

constexpr auto MAX_REGNAME_SIZE = 1024; // real max size - 32767 bytes;
constexpr auto MAX_REGVAL_SIZE = 1024 * 256; // real max size - 2 Mb;
constexpr auto REGISTRY_APP_ROOT = LR"(SOFTWARE\Dune IPTV Channel Editor)";
constexpr auto REGISTRY_APP_SETTINGS = LR"(SOFTWARE\Dune IPTV Channel Editor\Editor\Settings)";
constexpr auto CONFIG_FILE = L"settings.cfg";

static std::set<std::wstring> all_settings_keys = {
	REG_WINDOW_POS,
	REG_ICON_WINDOW_POS,
	REG_EPG_WINDOW_POS,
	REG_ACC_WINDOW_POS,
	REG_PLUGIN_CFG_WINDOW_POS,
	REG_PLAYER,
	REG_FFPROBE,
	REG_LISTS_PATH,
	REG_OUTPUT_PATH,
	REG_WEB_UPDATE_PATH,
	REG_SAVE_SETTINGS_PATH,
	REG_AUTO_SYNC,
	REG_AUTO_HIDE,
	REG_MAX_THREADS,
	REG_LANGUAGE,
	REG_CMP_FLAGS,
	REG_UPDATE_FREQ,
	REG_UPDATE_PL,
	REG_COLOR_ADDED,
	REG_COLOR_NOT_ADDED,
	REG_COLOR_CHANGED,
	REG_COLOR_HEVC,
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
	REG_VPORTAL,
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
	REG_PROFILE_ID,
	REG_SHOW_URL,
	REG_SHOW_EPG,
	REG_CREDENTIALS,
	REG_EMBED_INFO,
	REG_LIST_DOMAIN,
	REG_EPG_DOMAIN,
	REG_PLUGIN_SUFFIX,
	REG_ACCOUNT_DATA,
	REG_ACTIVE_ACCOUNT,
	REG_ACTIVE_CH_LIST,
};

static std::vector<PluginType> all_plugins = {
	{ PluginType::enAntifriz,   },
	{ PluginType::enCbilling,   },
	{ PluginType::enEdem,       },
	{ PluginType::enFilmax,     },
	{ PluginType::enFox,        },
	{ PluginType::enGlanz,      },
	{ PluginType::enIptvOnline, },
	{ PluginType::enItv,        },
	{ PluginType::enKineskop,   },
	{ PluginType::enLightIptv,  },
	{ PluginType::enMymagic,    },
	{ PluginType::enOneCent,    },
	{ PluginType::enOneOtt,     },
	{ PluginType::enOneUsd,     },
	{ PluginType::enOttclub,    },
	{ PluginType::enPing,       },
	{ PluginType::enRusskoeTV,  },
	{ PluginType::enSharaTV,    },
	{ PluginType::enSharaclub,  },
	{ PluginType::enSharavoz,   },
	{ PluginType::enShuraTV,    },
	{ PluginType::enSmile,      },
	{ PluginType::enTVClub,     },
	{ PluginType::enTvTeam,     },
	{ PluginType::enVidok,      },
	{ PluginType::enVipLime,    },
	{ PluginType::enCustom,     },
};

void AccountSettings::SaveSettings()
{
	if (m_bPortable)
		SaveSettingsToJson();
	else
		SaveSettingsToRegistry();
}

void AccountSettings::LoadSettings()
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
			ReadSettingsJson(PluginType::enBase);
			for (const auto& plugin : all_plugins)
			{
				ReadSettingsJson(plugin);
			}

			m_bPortable = TRUE;
		}

	}

	if (!m_bPortable)
	{
		m_settings.clear();
		ReadSettingsRegistry(PluginType::enBase);
		for (const auto& plugin : all_plugins)
		{
			m_pluginType = plugin;
			ReadSettingsRegistry(m_pluginType);
		}
	}

	int idx = get_plugin_idx();
	m_pluginType = (idx >= 0 && idx < (int)all_plugins.size()) ? all_plugins[idx] : PluginType::enEdem;
}

void AccountSettings::SaveSettingsToJson()
{
	UpdateSettingsJson(PluginType::enBase);

	for (const auto& plugin : all_plugins)
	{
		UpdateSettingsJson(plugin);
	}

	std::ofstream out_file(GetAppPath() + CONFIG_FILE);
	out_file << m_config << std::endl;
}

void AccountSettings::SaveSettingsToRegistry()
{
	SaveSectionRegistry(PluginType::enBase);

	for (const auto& plugin : all_plugins)
	{
		SaveSectionRegistry(plugin);
	}
}

void AccountSettings::UpdatePluginSettings()
{
	if (m_bPortable)
		UpdateSettingsJson(m_pluginType);
	else
		SaveSectionRegistry(m_pluginType);
}

void AccountSettings::RemovePortableSettings()
{
	std::error_code ec;
	std::filesystem::remove(GetAppPath() + CONFIG_FILE, ec);
}

const std::vector<PluginType>& AccountSettings::get_all_plugins() const
{
	return all_plugins;
}

int AccountSettings::get_plugin_idx() const
{
	return get_int(true, REG_PLUGIN);
}

void AccountSettings::set_plugin_idx(int val)
{
	set_int(true, REG_PLUGIN, val);
	m_pluginType = val < (int)all_plugins.size() ? all_plugins[val] : PluginType::enEdem;
}

PluginType AccountSettings::get_plugin_type() const
{
	return m_pluginType;
}

void AccountSettings::set_plugin_type(PluginType val)
{
	m_pluginType = val;
}

std::wstring AccountSettings::get_string(bool isApp, const std::wstring& key, const wchar_t* def /*= L""*/) const
{
	const auto& section = isApp ? PluginType::enBase : m_pluginType;

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

void AccountSettings::set_string(bool isApp, const std::wstring& key, const std::wstring& value)
{
	auto& settings = isApp ? m_settings[PluginType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

int AccountSettings::get_int(bool isApp, const std::wstring& key, const int def /*= 0*/) const
{
	const auto& section = isApp ? PluginType::enBase : m_pluginType;
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

void AccountSettings::set_int(bool isApp, const std::wstring& key, const int value)
{
	auto& settings = isApp ? m_settings[PluginType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

__int64 AccountSettings::get_int64(bool isApp, const std::wstring& key, const __int64 def /*= 0*/) const
{
	const auto& section = isApp ? PluginType::enBase : m_pluginType;
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

void AccountSettings::set_int64(bool isApp, const std::wstring& key, const __int64 value)
{
	auto& settings = isApp ? m_settings[PluginType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

std::vector<BYTE> AccountSettings::get_binary(bool isApp, const std::wstring& key) const
{
	const auto& section = isApp ? PluginType::enBase : m_pluginType;
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

bool AccountSettings::get_binary(bool isApp, const std::wstring& key, LPBYTE* pbData, size_t& dwSize) const
{
	const auto& section = isApp ? PluginType::enBase : m_pluginType;
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

void AccountSettings::set_binary(bool isApp, const std::wstring& key, const std::vector<BYTE>& value)
{
	auto& settings = isApp ? m_settings[PluginType::enBase] : m_settings[m_pluginType];

	settings[key] = value;
}

void AccountSettings::set_binary(bool isApp, const std::wstring& key, const BYTE* value, const size_t value_size)
{
	auto& settings = isApp ? m_settings[PluginType::enBase] : m_settings[m_pluginType];

	settings[key] = std::move(std::vector<BYTE>(value, value + value_size));
}

void AccountSettings::delete_setting(bool isApp, const std::wstring& key)
{
	auto& settings = isApp ? m_settings[PluginType::enBase] : m_settings[m_pluginType];
	settings.erase(key);
}

void AccountSettings::ReadSettingsRegistry(PluginType plugin_type)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_READ, &hkHive) != ERROR_SUCCESS)
		return;

	const auto& reg_key = fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetPluginShortNameW(plugin_type, false));
	HKEY hKey = nullptr;
	if (::RegOpenKeyExW(hkHive, reg_key.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		auto& settings = m_settings[plugin_type];
		DWORD dwIdx = 0;
		for (;;)
		{
			DWORD dwType = 0;
			DWORD cbData = MAX_REGVAL_SIZE;
			DWORD dwNameSize = MAX_REGNAME_SIZE / sizeof(wchar_t);

			std::vector<wchar_t> szName(dwNameSize);
			std::vector<BYTE> lpData(cbData);

			if (::RegEnumValueW(hKey, dwIdx++, (LPWSTR)szName.data(), &dwNameSize, nullptr, &dwType, (BYTE*)lpData.data(), &cbData) != ERROR_SUCCESS) break;

			std::wstring name(szName.data(), dwNameSize);
			if (all_settings_keys.find(name) == all_settings_keys.end()) // ignore unknown keys
			{
				TRACE(L"Unknown key: %s", name.c_str());
				continue;
			}

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

void AccountSettings::SaveSectionRegistry(PluginType plugin_type)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_WRITE, &hkHive) != ERROR_SUCCESS)
		return;

	const auto& reg_key = fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetPluginShortNameW(plugin_type, false));
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

bool AccountSettings::ReadSettingsJson(PluginType plugin_type)
{
	std::string j_section;
	if (plugin_type == PluginType::enBase)
		j_section = utils::utf16_to_utf8(std::wstring_view(APP_SETTINGS));
	else
		j_section = GetPluginShortNameA(plugin_type, false);

	if (!m_config.contains(j_section))
		return false;

	auto& settings = m_settings[plugin_type];
	const auto& node = m_config[j_section];
	for (const auto& item : node.items())
	{
		const auto& name = utils::utf8_to_utf16(item.key());
		if (all_settings_keys.find(name) == all_settings_keys.end()) // ignore unknown keys
		{
			TRACE(L"Unknown key: %s", name.c_str());
			continue;
		}

		switch (item.value().type())
		{
			case nlohmann::detail::value_t::number_integer:
			case nlohmann::detail::value_t::number_unsigned:
				settings[name] = item.value().get<int>();
				break;
			case nlohmann::detail::value_t::object:
				if (item.value().contains("qw"))
				{
					auto qw(item.value()["qw"]);
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

void AccountSettings::UpdateSettingsJson(PluginType plugin_type)
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

	if (plugin_type == PluginType::enBase)
		m_config[utils::utf16_to_utf8(std::wstring_view(APP_SETTINGS))] = node;
	else
		m_config[GetPluginShortNameA(plugin_type, false)] = node;
}
