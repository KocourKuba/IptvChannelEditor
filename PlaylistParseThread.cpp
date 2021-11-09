// PlaylistParseThread.cpp : implementation file
//

#include "stdafx.h"
#include "PlaylistParseThread.h"
#include "utils.h"

void CPlaylistParseThread::ThreadConfig::NotifyParent(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_parent->GetSafeHwnd())
		m_parent->SendMessage(message, wParam, lParam);

}

IMPLEMENT_DYNCREATE(CPlaylistParseThread, CWinThread)

BOOL CPlaylistParseThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	auto entries = std::make_unique<std::vector<std::shared_ptr<PlaylistEntry>>>();
	if (m_config.m_data)
	{
		utils::vector_to_streambuf<char> buf(*m_config.m_data);
		std::istream stream(&buf);
		if (stream.good())
		{
			int step = 0;
			auto entry = std::make_shared<PlaylistEntry>(m_config.m_pluginType, m_config.m_rootPath);
			std::string line;
			int count = 0;
			std::string logo_root;
			while (std::getline(stream, line))
			{
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
				{
					entries.reset();
					break;
				}

				utils::string_rtrim(line, "\r");
				count++;

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

				if (m_config.m_pluginType == StreamType::enSharavoz)
				{
					entry->set_epg1_id(entry->stream_uri->get_id()); // primary EPG
					entry->set_epg2_id(entry->get_epg1_id()); // secondary EPG
				}

				if (m_config.m_pluginType == StreamType::enOneUsd || m_config.m_pluginType == StreamType::enTvTeam)
				{
					entry->set_epg1_id(entry->get_title()); // primary EPG
				}

				m_config.NotifyParent(WM_UPDATE_PROGRESS, step++, count);
				entries->emplace_back(entry);
				entry = std::make_shared<PlaylistEntry>(m_config.m_pluginType, m_config.m_rootPath);
			}
		}
	}

	m_config.NotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)entries.release(), 0);

	CoUninitialize();

	return FALSE;
}
