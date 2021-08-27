#pragma once

#include "PlayListEntry.h"

#define WM_UPDATE_PROGRESS (WM_USER + 301)
#define WM_END_LOAD_PLAYLIST (WM_USER + 302)

class CPlaylistParseThread : public CWinThread
{
	DECLARE_DYNCREATE(CPlaylistParseThread)
public:
	class ThreadConfig
	{
	public:
		void NotifyParent(UINT message, WPARAM wParam);

		std::vector<BYTE>* m_data = nullptr;
		CWnd* m_parent = nullptr;
		HANDLE m_hStop = nullptr;
		StreamType m_pluginType = StreamType::enEdem;
		std::wstring m_rootPath;
	};

protected:
	CPlaylistParseThread() { m_bAutoDelete = TRUE;  }

public:
	virtual ~CPlaylistParseThread() { delete m_config.m_data; }

public:
	BOOL InitInstance() override;

	void SetData(const ThreadConfig& config) { m_config = config; };

protected:
	ThreadConfig m_config;
	std::wregex m_re;
};
