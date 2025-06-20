/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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

#include "pch.h"
#include "AccountSettings.h"
#include "Constants.h"
#include "IPTVChannelEditor.h"
#include "Credentials.h"
#include "PluginFactory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAKEQWORD(a, b) ((QWORD)(((DWORD)(((QWORD)(a)) & 0xffffffff)) | ((QWORD)((DWORD)(((QWORD)(b)) & 0xffffffff))) << 32))

constexpr auto MAX_REGNAME_SIZE = 1024; // real max size - 32767 bytes;
constexpr auto MAX_REGVAL_SIZE = 1024 * 256; // real max size - 2 Mb;
constexpr auto REGISTRY_APP_ROOT = LR"(SOFTWARE\Dune IPTV Channel Editor)";
constexpr auto REGISTRY_APP_SETTINGS = LR"(SOFTWARE\Dune IPTV Channel Editor\Editor\Settings)";
constexpr auto CONFIG_FILE = L"settings.cfg";
constexpr const char* const common_settings = "common_settings";

static std::set<std::wstring> all_settings_keys = {
	REG_WINDOW_POS,
	REG_ICON_WINDOW_POS,
	REG_ICON_COLUMNS_WIDTH,
	REG_EPG_WINDOW_POS,
	REG_EPG_COLUMNS_WIDTH,
	REG_ACC_WINDOW_POS,
	REG_PLUGIN_CFG_WINDOW_POS,
	REG_FILL_INFO_WINDOW_POS,
	REG_PLAYER,
	REG_FFPROBE,
	REG_LISTS_PATH,
	REG_OUTPUT_PATH,
	REG_WEB_UPDATE_PATH,
	REG_SAVE_SETTINGS_PATH,
	REG_SAVE_IMAGE_PATH,
	REG_AUTO_SYNC,
	REG_AUTO_HIDE,
	REG_CONVERT_DUPES,
	REG_MAX_THREADS,
	REG_MAX_CACHE_TTL,
	REG_LANGUAGE,
	REG_CMP_FLAGS,
	REG_UPDATE_FREQ,
	REG_UPDATE_PL,
	REG_UPDATE_SERVER,
	REG_COLOR_ADDED,
	REG_COLOR_NOT_ADDED,
	REG_COLOR_CHANGED,
	REG_COLOR_UNKNOWN,
	REG_COLOR_HEVC,
	REG_COLOR_DUPLICATED,
	REG_PLUGIN,
	REG_SEL_PLUGIN,
	REG_ICON_SOURCE,
	REG_DAYS_BACK,
	REG_HOURS_BACK,
	REG_NEXT_UPDATE,
	REG_AVAIL_UPDATE,
	REG_DUNE_IP,
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
	REG_CUSTOM_PL_FILE,
	REG_DEVICE_ID,
	REG_PROFILE_ID,
	REG_SHOW_URL,
	REG_SHOW_EPG,
	REG_CREDENTIALS,
	REG_EMBED_INFO,
	REG_PLUGIN_SUFFIX,
	REG_ACCOUNT_DATA,
	REG_ACTIVE_ACCOUNT,
	REG_ACTIVE_CH_LIST,
	REG_CUSTOM_XMLTV_SOURCE,
	REG_EPG_ID_IDX,
	REG_EPG_SOURCE_IDX
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
		JSON_ALL_TRY;
		{
			in_file >> m_config;
		}
		JSON_ALL_CATCH;

		if (!m_config.empty())
		{
			ReadSettingsJson(common_settings);
			for (const auto& pair : GetPluginFactory().get_all_configs())
			{
				ReadSettingsJson(pair.first);
			}

			m_bPortable = TRUE;
		}

	}

	if (!m_bPortable)
	{
		m_settings.clear();
		ReadSettingsRegistry(common_settings);
		for (const auto& pair : GetPluginFactory().get_all_configs())
		{
			m_pluginType = pair.first;
			ReadSettingsRegistry(m_pluginType);
		}
	}

	m_pluginType = get_selected_plugin();
	if (m_pluginType.empty())
	{
		m_pluginType = GetPluginFactory().get_all_configs().begin()->first;
	}
}

void AccountSettings::SaveSettingsToJson()
{
	UpdateSettingsJson(common_settings);

	for (const auto& pair : GetPluginFactory().get_all_configs())
	{
		UpdateSettingsJson(pair.first);
	}

	std::ofstream out_file(GetAppPath() + CONFIG_FILE, std::ofstream::binary);
	out_file << m_config << std::endl;
}

void AccountSettings::SaveSettingsToRegistry()
{
	SaveSectionRegistry(common_settings);

	for (const auto& pair : GetPluginFactory().get_all_configs())
	{
		SaveSectionRegistry(pair.first);
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

std::string AccountSettings::get_selected_plugin() const
{
	return utils::utf16_to_utf8(get_string(true, REG_SEL_PLUGIN));
}

void AccountSettings::set_selected_plugin(const std::string& val)
{
	set_string(true, REG_SEL_PLUGIN, utils::utf8_to_utf16(val));
	set_plugin_type(val);
}

const std::string& AccountSettings::get_plugin_type() const
{
	return m_pluginType;
}

void AccountSettings::set_plugin_type(const std::string& val)
{
	m_pluginType = val;
}

std::vector<Credentials> AccountSettings::LoadCredentials() const
{
	std::vector<Credentials> credentials;
	nlohmann::json creds;
	JSON_ALL_TRY;
	{
		const auto& data = GetConfig().get_string(false, REG_ACCOUNT_DATA);
		if (!data.empty())
		{
			creds = nlohmann::json::parse(data);
		}
	}
	JSON_ALL_CATCH;
	for (const auto& item : creds.items())
	{
		const auto& val = item.value();
		if (val.empty()) continue;

		Credentials cred;
		JSON_ALL_TRY;
		{
			cred = val.get<Credentials>();
			// only edem has special cred type and only edem has server filters
			// it's not correctly but using this method
			if (GetPluginFactory().get_config(m_pluginType).get_vod_server_filter() && (cred.ott_key.empty())) {
				std::swap(cred.ott_key, cred.token);
				std::swap(cred.subdomain, cred.domain);
			}
		}
		JSON_ALL_CATCH;
		credentials.emplace_back(cred);
	}

	return credentials;
}

std::wstring AccountSettings::get_string(bool isApp, const std::wstring& key, const wchar_t* def /*= L""*/) const
{
	const auto& section = isApp ? common_settings : m_pluginType;

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
	auto& settings = isApp ? m_settings[common_settings] : m_settings[m_pluginType];

	settings[key] = value;
}

int AccountSettings::get_int(bool isApp, const std::wstring& key, const int def /*= 0*/) const
{
	const auto& section = isApp ? common_settings : m_pluginType;
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
	auto& settings = isApp ? m_settings[common_settings] : m_settings[m_pluginType];

	settings[key] = value;
}

__int64 AccountSettings::get_int64(bool isApp, const std::wstring& key, const __int64 def /*= 0*/) const
{
	const auto& section = isApp ? common_settings : m_pluginType;
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
	auto& settings = isApp ? m_settings[common_settings] : m_settings[m_pluginType];

	settings[key] = value;
}

std::vector<BYTE> AccountSettings::get_binary(bool isApp, const std::wstring& key) const
{
	const auto& section = isApp ? common_settings : m_pluginType;
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
	const auto& section = isApp ? common_settings : m_pluginType;
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
	auto& settings = isApp ? m_settings[common_settings] : m_settings[m_pluginType];

	settings[key] = value;
}

void AccountSettings::set_binary(bool isApp, const std::wstring& key, const BYTE* value, const size_t value_size)
{
	auto& settings = isApp ? m_settings[common_settings] : m_settings[m_pluginType];

	settings[key] = std::move(std::vector<BYTE>(value, value + value_size));
}

void AccountSettings::delete_setting(bool isApp, const std::wstring& key)
{
	auto& settings = isApp ? m_settings[common_settings] : m_settings[m_pluginType];
	settings.erase(key);
}

void AccountSettings::ReadSettingsRegistry(const std::string& plugin_type)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_READ, &hkHive) != ERROR_SUCCESS)
		return;

	const auto& reg_key = fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetPluginTypeNameW(plugin_type));
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
				TRACE(L"Unknown key: %s\n", name.c_str());
				continue;
			}

			switch (dwType)
			{
				case REG_DWORD:
					settings[name] = *reinterpret_cast<int*>(lpData.data());
					break;
				case REG_QWORD:
					settings[name] = *reinterpret_cast<__int64*>(lpData.data());
					break;
				case REG_SZ:
					settings[name] = reinterpret_cast<wchar_t*>(lpData.data());
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

void AccountSettings::SaveSectionRegistry(const std::string& plugin_type)
{
	HKEY hkHive = nullptr;
	if (::RegOpenCurrentUser(KEY_WRITE, &hkHive) != ERROR_SUCCESS)
		return;

	const auto& reg_key = fmt::format(LR"({:s}\{:s})", REGISTRY_APP_SETTINGS, GetPluginTypeNameW(plugin_type));
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
					const auto pbData = reinterpret_cast<const BYTE*>(&std::get<int>(pair.second));
					::RegSetValueExW(hKey, pair.first.c_str(), 0, REG_DWORD, pbData, sizeof(int));
					break;
				}
				case 1: // __int64
				{
					const auto pbData = reinterpret_cast<const BYTE*>(&std::get<__int64>(pair.second));
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

bool AccountSettings::ReadSettingsJson(const std::string& plugin_type)
{
	std::string j_section;
	if (plugin_type == common_settings)
		j_section = utils::utf16_to_utf8(std::wstring_view(APP_SETTINGS));
	else
		j_section = GetPluginTypeNameA(plugin_type, false);

	if (!m_config.contains(j_section))
		return false;

	auto& settings = m_settings[plugin_type];
	const auto& node = m_config[j_section];
	for (const auto& item : node.items())
	{
		const auto& name = utils::utf8_to_utf16(item.key());
		if (all_settings_keys.find(name) == all_settings_keys.end()) // ignore unknown keys
		{
			TRACE(L"Unknown key: %s\n", name.c_str());
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

void AccountSettings::UpdateSettingsJson(const std::string& plugin_type)
{
	auto& settings = m_settings[plugin_type];
	nlohmann::json node;
	for (const auto& pair : settings)
	{
		const auto& name = utils::utf16_to_utf8(pair.first);
		JSON_ALL_TRY;
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
		JSON_ALL_CATCH;
	}

	if (plugin_type == common_settings)
		m_config[utils::utf16_to_utf8(std::wstring_view(APP_SETTINGS))] = node;
	else
		m_config[GetPluginTypeNameA(plugin_type, false)] = node;
}
