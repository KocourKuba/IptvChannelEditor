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
		const auto& wbuf = utils::utf8_to_utf16(m_config.m_data.str());
		std::wistringstream stream(wbuf);
		if (stream.good())
		{
			m_config.SendNotifyParent(WM_INIT_PROGRESS, (int)std::count(wbuf.begin(), wbuf.end(), '\n'), 0);

			auto entry = std::make_shared<PlaylistEntry>(m_parent_plugin, m_config.m_rootPath);

			int channels = 0;
			int step = 0;

			std::wstring line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, L"\r");
				step++;

				if (entry->Parse(line))
				{
					if (entry->get_id().empty())
					{
						entry->search_id(m_parent_plugin->get_tag_id_match());
						if (entry->get_id().empty())
						{
							entry->set_is_template(false);
						}
					}

					playlist->m_entries.emplace_back(entry);
					entry = std::make_shared<PlaylistEntry>(m_parent_plugin, m_config.m_rootPath);
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
			}
		}
	}

	m_config.SendNotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)playlist.release());

	::SetEvent(m_config.m_hExit);
	ATLTRACE("\nThread exit\n");

	CoUninitialize();

	return FALSE;
}
