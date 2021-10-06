#pragma once

#include "GetStreamInfoThread.h"
#include "uri_stream.h"

class CGetStreamInfoThread : public CWinThread
{
	DECLARE_DYNCREATE(CGetStreamInfoThread)
public:
	class ThreadConfig
	{
	public:
		void NotifyParent(UINT message, WPARAM wParam, LPARAM lParam);

		std::vector<uri_stream*>* m_container = nullptr;
		CWnd* m_parent = nullptr;
		HANDLE m_hStop = nullptr;
		CString m_probe;
		bool m_isChannelsTree = true;
		TemplateParams m_params;
		StreamSubType m_StreamSubtype = StreamSubType::enHLS;
	};

protected:
	CGetStreamInfoThread() { m_bAutoDelete = TRUE;  }

public:
	virtual ~CGetStreamInfoThread() { delete m_config.m_container; }

public:
	BOOL InitInstance() override;

	void SetData(const ThreadConfig& config) { m_config = config; };

protected:
	static void GetChannelStreamInfo(const CString& probe, const std::string& url, std::string& audio, std::string& video);

protected:
	ThreadConfig m_config;
	std::wregex m_re;
};
