#pragma once

#include "Config.h"

class CPlaylistParseXMLThread : public CWinThread
{
	DECLARE_DYNCREATE(CPlaylistParseXMLThread)

protected:
	CPlaylistParseXMLThread() { m_bAutoDelete = TRUE; }

public:
	virtual ~CPlaylistParseXMLThread() { delete m_config.m_data; }

public:
	BOOL InitInstance() override;

	void SetData(const ThreadConfig& config) { m_config = config; };

protected:
	ThreadConfig m_config;
	std::wregex m_re;
};
