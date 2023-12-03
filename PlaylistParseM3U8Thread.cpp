/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlaylistParseM3U8Thread, CWinThread)

BOOL CPlaylistParseM3U8Thread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	auto playlist = std::make_unique<Playlist>();
	if (m_config.m_data)
	{
		const auto& wbuf = m_config.m_data.str();
		std::istringstream stream(wbuf);
		if (stream.good())
		{
			m_config.SendNotifyParent(WM_INIT_PROGRESS, (int)std::count(wbuf.begin(), wbuf.end(), '\n'), 0);

			auto entry = std::make_shared<PlaylistEntry>(m_parent_plugin, playlist, m_config.m_rootPath);
			const auto& pl_info = m_parent_plugin->get_current_playlist_info();
			int channels = 0;
			int step = 0;

			std::string line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, "\r");
				step++;

				if (entry->Parse(line))
				{
					if (entry->get_id().empty())
					{
						entry->search_id(pl_info.get_tag_id_match());
						if (entry->get_id().empty())
						{
							entry->set_is_template(false);
							entry->recalc_hash();
						}
					}

					switch (m_parent_plugin->get_plugin_type())
					{
						case PluginType::enOttclub:
							entry->set_icon_uri(fmt::format(L"http://{:s}/images/{:s}.png", entry->get_domain(), entry->get_id()));
							break;
						case PluginType::enKineskop:
							entry->set_icon_uri(boost::regex_replace(entry->get_icon_uri().get_uri(), boost::wregex(LR"(http:\/\/\w{2}\.(.*))"), L"http://$1"));
							break;
						default:
							break;
					}

					// special cases after parsing
					if (pl_info.get_epg_id_from_id())
					{
						entry->set_epg_id(0, entry->get_id());
					}

					if (!m_parent_plugin->get_epg_parameter(1).epg_url.empty())
					{
						entry->set_epg_id(1, entry->get_epg_id(0));
						if (m_parent_plugin->get_plugin_type() == PluginType::enIptvOnline && entry->get_epg_id(0).front() == 'X')
						{
							entry->set_epg_id(0, entry->get_epg_id(0).substr(1));
						}
					}

					playlist->m_entries.emplace_back(entry);
					entry = std::make_shared<PlaylistEntry>(m_parent_plugin, playlist, m_config.m_rootPath);

					channels++;
					if (channels % 100 == 0)
					{
						m_config.SendNotifyParent(WM_UPDATE_PROGRESS, channels, step);
						if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
						{
							playlist.reset();
							break;
						}
					}
				}

				if (entry->get_m3u_entry().get_directive() == m3u_entry::directives::ext_header)
				{
					entry = std::make_shared<PlaylistEntry>(m_parent_plugin, playlist, m_config.m_rootPath);
				}
			}
		}
	}

	m_config.SendNotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)playlist.release());

	::SetEvent(m_config.m_hExit);
	ATLTRACE("\nThread exit\n");

	CoUninitialize();

	return FALSE;
}
