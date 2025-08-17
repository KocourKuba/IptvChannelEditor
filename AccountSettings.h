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

#pragma once
#include <map>
#include <variant>

#include "nlohmann\json.hpp"

class Credentials;

class AccountSettings
{
public:
	static AccountSettings& Instance()
	{
		static AccountSettings _instance;
		return _instance;
	}
	using map_variant = std::map<std::wstring, std::variant<int, __int64, std::wstring, std::vector<unsigned char>>>;

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

	std::string get_selected_plugin() const;
	void set_selected_plugin(const std::string& val);

	const std::string& get_plugin_type() const;
	void set_plugin_type(const std::string& val);

	BOOL IsPortable() const { return m_bPortable; }
	void SetPortable(BOOL val) { m_bPortable = val; }

	std::vector<Credentials> LoadCredentials() const;

public:
	std::wstring get_string(bool isApp, const std::wstring& key, const wchar_t* def = L"") const;
	void set_string(bool isApp, const std::wstring& key, const std::wstring& value);

	int get_int(bool isApp, const std::wstring& key, const int def = 0) const;
	void set_int(bool isApp, const std::wstring& key, const int value);

	__int64 get_int64(bool isApp, const std::wstring& key, const __int64 def = 0) const;
	void set_int64(bool isApp, const std::wstring& key, const __int64 value);

	std::vector<unsigned char> get_binary(bool isApp, const std::wstring& key) const;

	void set_binary(bool isApp, const std::wstring& key, const std::span<unsigned char>& value);

	void delete_setting(bool isApp, const std::wstring& key);

	template <class T = std::chrono::hours>
	T get_chrono(bool isApp, const std::wstring& key, const T& def = T::zero()) const
	{
		return T(get_int(isApp, key, def.count()));
	}

	template <class T = std::chrono::hours>
	void set_chrono(bool isApp, const std::wstring& key, const T& val)
	{
		set_int(isApp, key, val.count());
	}

protected:
	void ReadSettingsRegistry(const std::string& plugin_type);
	void SaveSectionRegistry(const std::string& plugin_type);

	bool ReadSettingsJson(const std::string& plugin_type);
	void UpdateSettingsJson(const std::string& plugin_type);

private:
	BOOL m_bPortable = FALSE;
	std::map<std::string, map_variant> m_settings{};
	std::string m_pluginType;
	nlohmann::json m_config{};
};

inline AccountSettings& GetConfig() { return AccountSettings::Instance(); }
