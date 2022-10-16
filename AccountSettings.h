#pragma once
#include <map>
#include <variant>
#include "PluginDefines.h"

#include "UtilsLib\json_wrapper.h"

class AccountSettings
{
public:
	static AccountSettings& Instance()
	{
		static AccountSettings _instance;
		return _instance;
	}
	using map_variant = std::map<std::wstring, std::variant<int, __int64, std::wstring, std::vector<BYTE>>>;

private:
	AccountSettings() = default;
	~AccountSettings() = default;

	AccountSettings(const AccountSettings& source) = delete;

public:
	void SaveSettings();
	void LoadSettings();

	void SaveSettingsToJson();
	void SaveSettingsToRegistry();

	void UpdatePluginSettings();

	void RemovePortableSettings();

	const std::vector<PluginType>& get_all_plugins() const;

	int get_plugin_idx() const;
	void set_plugin_idx(int val);

	PluginType get_plugin_type() const;
	void set_plugin_type(PluginType val);

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
	void ReadSettingsRegistry(PluginType plugin_type);
	void SaveSectionRegistry(PluginType plugin_type);

	bool ReadSettingsJson(PluginType plugin_type);
	void UpdateSettingsJson(PluginType plugin_type);

private:
	BOOL m_bPortable = FALSE;
	std::map<PluginType, map_variant> m_settings;
	PluginType m_pluginType = PluginType::enEdem;
	nlohmann::json m_config;
};

inline AccountSettings& GetConfig() { return AccountSettings::Instance(); }
