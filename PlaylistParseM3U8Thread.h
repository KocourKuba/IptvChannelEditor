#pragma once

#include "Config.h"

class CPlaylistParseM3U8Thread : public CWinThread
{
	DECLARE_DYNCREATE(CPlaylistParseM3U8Thread)
public:
	class ThreadConfig
	{
	public:
		void NotifyParent(UINT message, WPARAM wParam, LPARAM lParam);

		std::vector<BYTE>* m_data = nullptr;
		CWnd* m_parent = nullptr;
		HANDLE m_hStop = nullptr;
		StreamType m_pluginType = StreamType::enEdem;
		std::wstring m_rootPath;
	};

protected:
	CPlaylistParseM3U8Thread() { m_bAutoDelete = TRUE; }

public:
	virtual ~CPlaylistParseM3U8Thread() { delete m_config.m_data; }

public:
	BOOL InitInstance() override;

	void SetData(const ThreadConfig& config) { m_config = config; };

protected:
	ThreadConfig m_config;
	std::wregex m_re;
};
