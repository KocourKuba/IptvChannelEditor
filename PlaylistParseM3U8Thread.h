#pragma once

#include "Config.h"

class CPlaylistParseM3U8Thread : public CWinThread
{
	DECLARE_DYNCREATE(CPlaylistParseM3U8Thread)

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
