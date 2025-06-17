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
#include "base_plugin.h"
#include "IconPackInfo.h"

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

	std::shared_ptr<base_plugin> create_plugin(const std::string& type);

	bool load_configs(bool dev = false);

	EpgParameters get_epg_preset(const std::string& name) const;
	const std::map<std::string, EpgParameters>& get_epg_presets() const;

	IconPackInfo get_icon_pack_info(const size_t idx) const;
	const std::vector<IconPackInfo>& get_icon_packs() const;

	const std::map<std::string, plugin_config>& get_all_configs() const;
	const plugin_config get_config(const std::string& type) const;

protected:
	PluginFactory() = default;
	virtual ~PluginFactory() = default;

private:

	PluginFactory(const PluginFactory& source) = delete;

private:
	std::map<std::string, plugin_config> m_config_storage;
	std::map<std::string, EpgParameters> m_known_presets;
	std::vector<IconPackInfo> m_image_libs;
};

inline PluginFactory& GetPluginFactory() { return PluginFactory::Instance(); }
