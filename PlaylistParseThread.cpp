// PlaylistParseThread.cpp : implementation file
//

#include "stdafx.h"
#include "PlaylistParseThread.h"
#include "utils.h"

// COnlineCheckActivation


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
			while (std::getline(stream, line))
			{
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
				{
					entries.reset();
					break;
				}

				utils::string_rtrim(line, "\r");
				count++;
				if (!line.empty() && entry->Parse(line))
				{
					if (m_config.m_pluginType == StreamType::enSharavoz)
					{
						entry->set_epg1_id(entry->get_stream_uri()->get_id()); // primary EPG
						entry->set_epg2_id(entry->get_epg1_id()); // secondary EPG
					}

					m_config.NotifyParent(WM_UPDATE_PROGRESS, step++, count);
					entries->emplace_back(entry);
					entry = std::make_shared<PlaylistEntry>(m_config.m_pluginType, m_config.m_rootPath);
				}
			}
		}
	}

	m_config.NotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)entries.release(), 0);

	CoUninitialize();

	return FALSE;
}
