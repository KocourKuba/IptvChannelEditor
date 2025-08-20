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
#include "PlaylistParseM3U8Thread.h"
#include "PlayListEntry.h"
#include "base_plugin.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void PlaylistParseM3U8Thread(const std::shared_ptr<ThreadConfig> config, const std::shared_ptr<base_plugin> parent_plugin, const std::wstring& rootPath)
{
	auto playlist = std::make_unique<Playlist>();
	if (config->m_data && parent_plugin)
	{
		const auto& wbuf = config->m_data->str();
		std::istringstream stream(wbuf);
		if (stream.good())
		{
			SendNotifyParent(config->m_parent, WM_INIT_PROGRESS, (int)std::count(wbuf.begin(), wbuf.end(), '\n'), 0);

			auto entry = std::make_shared<PlaylistEntry>(playlist, rootPath);
			const auto& pl_info = parent_plugin->get_current_playlist_info();
			const auto& epg_info = parent_plugin->get_epg_parameters();
			int channels = 0;
			int step = 0;

			std::string line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, "\r");
				step++;

				if (entry->Parse(line))
				{
					parent_plugin->parse_stream_uri(utils::utf8_to_utf16(line), *entry);
					if (entry->is_valid() && entry->get_category().empty())
					{
						entry->set_category(load_string_resource(IDS_STRING_UNSET));
					}

					if (entry->get_id().empty())
					{
						entry->search_id(pl_info.get_tag_id_match());
						if (entry->get_id().empty())
						{
							entry->set_is_template(false);
							entry->recalc_hash();
						}
					}

					parent_plugin->update_entry(*entry);

					// special cases after parsing
					for (int i = 0; i < parent_plugin->get_epg_parameters().size(); ++i)
					{
						auto& epg_param = parent_plugin->get_epg_parameter(i);
						if (epg_param.epg_url.empty()) continue;

						switch (static_cast<epg_id_sources>(epg_param.get_epg_id_source()))
						{
							case epg_id_sources::enChannelId:
								entry->set_epg_id(i, entry->get_id());
								break;
							case epg_id_sources::enChannelName:
								entry->set_epg_id(i, entry->get_title());
								break;
							default:
								break;
						}
					}

					playlist->m_entries.emplace_back(entry);
					entry = std::make_shared<PlaylistEntry>(playlist, rootPath);

					channels++;
					if (channels % 100 == 0)
					{
						SendNotifyParent(config->m_parent, WM_UPDATE_PROGRESS, channels, step);
						if (::WaitForSingleObject(config->m_hStop, 0) == WAIT_OBJECT_0)
						{
							playlist.reset();
							break;
						}
					}
				}

				if (entry->get_m3u_entry().get_directive() == m3u_entry::directives::ext_header)
				{
					entry = std::make_shared<PlaylistEntry>(playlist, rootPath);
				}
			}
		}
	}

	SendNotifyParent(config->m_parent, WM_END_LOAD_PLAYLIST, (WPARAM)playlist.release());

	::SetEvent(config->m_hExit);
}
