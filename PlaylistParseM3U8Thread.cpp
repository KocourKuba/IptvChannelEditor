// PlaylistParseThread.cpp : implementation file
//

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
				if (m3uEntry.get_directive() == m3u_entry::ext_header)
				{
					const auto& tags = m3uEntry.get_tags();
					if (const auto& pair = tags.find(m3u_entry::tag_url_logo); pair != tags.end())
					{
						logo_root = pair->second;
					}
				}

				entry->set_logo_root(logo_root);
				if (!entry->Parse(line, m3uEntry)) continue;

				// special cases after parsing
				switch (m_config.m_pluginType)
				{
					case StreamType::enOneUsd:
					case StreamType::enTvTeam:
						entry->set_epg1_id(entry->get_title()); // primary EPG
						break;
					case StreamType::enSharavoz:
					case StreamType::enOneOtt:
					case StreamType::enCbilling:
					case StreamType::enShuraTV:
						entry->set_epg1_id(entry->stream_uri->get_id()); // primary EPG
						entry->set_epg2_id(entry->stream_uri->get_id()); // secondary EPG
						break;
					case StreamType::enLightIptv:
						entry->stream_uri->set_id(entry->get_epg1_id());
						break;
					case StreamType::enOttclub:
						entry->set_epg1_id(entry->stream_uri->get_id()); // primary EPG
						entry->set_icon_uri(fmt::format(L"http://{:s}/images/{:s}.png", entry->stream_uri->get_domain(), entry->stream_uri->get_id()));
						break;
					case StreamType::enIptvOnline:
						if (entry->get_epg1_id().front() == 'X')
							entry->set_epg1_id(entry->get_epg1_id().substr(1)); // primary EPG
						break;
					default:
						break;
				}

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

	m_config.NotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)playlist.release());

	CoUninitialize();

	return FALSE;
}
