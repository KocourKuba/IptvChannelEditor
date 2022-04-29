/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
		const auto& wbuf = utils::utf8_to_utf16((char*)m_config.m_data->data(), m_config.m_data->size());
		std::wistringstream stream(wbuf);
		if (stream.good())
		{
			m_config.NotifyParent(WM_INIT_PROGRESS, (int)std::count(wbuf.begin(), wbuf.end(), '\n'), TRUE);

			std::wstring logo_root;
			auto entry = std::make_shared<PlaylistEntry>(m_config.m_pluginType, m_config.m_rootPath);

			int channels = 0;
			int step = 0;

			std::wstring line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, L"\r");
				step++;

				m3u_entry m3uEntry(line);
				if (m3uEntry.get_directive() == m3u_entry::directives::ext_header)
				{
					const auto& tags = m3uEntry.get_tags();
					if (const auto& pair = tags.find(m3u_entry::info_tags::tag_url_logo); pair != tags.end())
					{
						logo_root = pair->second;
					}
				}

				entry->set_logo_root(logo_root);
				if (entry->Parse(line, m3uEntry))
				{
					playlist->m_entries.emplace_back(entry);
					entry = std::make_shared<PlaylistEntry>(m_config.m_pluginType, m_config.m_rootPath);
					channels++;

					if (channels % 50 == 0)
					{
						m_config.NotifyParent(WM_UPDATE_PROGRESS, channels, step);
						if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
						{
							playlist.reset();
							break;
						}
					}
				}
			}
		}
	}

	m_config.NotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)playlist.release());

	CoUninitialize();

	return FALSE;
}
