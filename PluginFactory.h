/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "base_plugin.h"

/// <summary>
/// Container for stream interface
/// </summary>
class PluginFactory
{
public:
	static PluginFactory& Instance()
	{
		static PluginFactory _instance;
		return _instance;
	}

	std::shared_ptr<base_plugin> create_plugin(PluginType type);
	EpgParameters get_epg_preset(EpgPresets idx) const;
	const std::array<EpgParameters, (size_t)EpgPresets::enCustom>& get_epg_presets() const
	{
		return known_presets;
	}

	bool load_configs();
	const std::vector<std::pair<PluginType, std::string>>& get_all_plugins() const;
	PluginType get_plugin_type(size_t idx);

protected:
	PluginFactory() = default;
	virtual ~PluginFactory() = default;

private:

	PluginFactory(const PluginFactory& source) = delete;

private:
	std::map<std::string, plugin_config> m_config_storage;
	std::array<EpgParameters, (size_t)EpgPresets::enCustom> known_presets{};
};

inline PluginFactory& GetPluginFactory() { return PluginFactory::Instance(); }