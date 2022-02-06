#pragma once

#include "uri_stream.h"

class CGetStreamInfoThread : public CWinThread
{
	DECLARE_DYNCREATE(CGetStreamInfoThread)
public:
	class ThreadConfig
	{
	public:
		void NotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) const;

		std::vector<uri_stream*>* m_container = nullptr;
		CWnd* m_parent = nullptr;
		HANDLE m_hStop = nullptr;
		std::wstring m_probe;
		int m_max_threads = 1;
		TemplateParams m_params;
		StreamSubType m_StreamSubtype = StreamSubType::enHLS;
	};

protected:
	CGetStreamInfoThread() { m_bAutoDelete = TRUE; }

public:
	virtual ~CGetStreamInfoThread() { delete m_config.m_container; }

public:
	BOOL InitInstance() override;

	void SetData(const ThreadConfig& config) { m_config = config; };

protected:
	static void GetChannelStreamInfo(const ThreadConfig& config, std::atomic<int>& count, int index);

protected:
	ThreadConfig m_config;
	std::wregex m_re;
};
