// PlaylistParseThread.cpp : implementation file
//

#include "stdafx.h"
#include "PlaylistParseThread.h"
#include "utils.h"

// COnlineCheckActivation


void CPlaylistParseThread::ThreadConfig::NotifyParent(UINT message, WPARAM wParam)
{
	if (m_parent->GetSafeHwnd())
		m_parent->SendMessage(message, wParam);

}

IMPLEMENT_DYNCREATE(CPlaylistParseThread, CWinThread)

BOOL CPlaylistParseThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	auto entries = std::make_unique<std::vector<std::unique_ptr<PlaylistEntry>>>();
	try
	{
		if (m_config.m_regex)
			m_re = m_config.m_filter.GetString();
	}
	catch (std::regex_error& ex)
	{
		ex;
		m_config.m_regex = false;
	}

	if (m_config.m_data)
	{
		utils::vector_to_streambuf<char> buf(*m_config.m_data);
		std::istream stream(&buf);
		if (stream.good())
		{
			int step = 0;
			auto entry = std::make_unique<PlaylistEntry>();
			std::string line;
			while (std::getline(stream, line))
			{
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
				{
					entries.reset();
					break;
				}

				m_config.NotifyParent(WM_UPDATE_PROGRESS, step++);

				utils::string_rtrim(line, "\r");
				if (line.empty()) continue;

				entry->Parse(line);
				if (entry->get_directive() == ext_pathname)
				{
					if (!filterEntry(entry.get()))
						entries->emplace_back(std::move(entry));

					entry = std::make_unique<PlaylistEntry>();
				}
			}
		}
	}

	m_config.NotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)entries.release());

	CoUninitialize();

	return FALSE;
}

bool CPlaylistParseThread::filterEntry(const PlaylistEntry* entry)
{
	if (m_config.m_filter.IsEmpty())
		return false;

	bool found = false;
	if (m_config.m_regex)
	{
		try
		{
			found = std::regex_search(entry->get_title(), m_re);
		}
		catch (std::regex_error& ex)
		{
			ex;
			found = true;
		}
	}
	else
	{
		if (m_config.m_case)
		{
			found = (entry->get_title().find(m_config.m_filter.GetString()) != std::wstring::npos);
		}
		else
		{
			found = (StrStrI(entry->get_title().c_str(), m_config.m_filter.GetString()) != nullptr);
		}
	}

	return found;
}
