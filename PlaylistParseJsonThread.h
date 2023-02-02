/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "base_plugin.h"
#include "ThreadConfig.h"

class CPlaylistParseJsonThread : public CWinThread
{
	DECLARE_DYNCREATE(CPlaylistParseJsonThread)

protected:
	CPlaylistParseJsonThread() { m_bAutoDelete = TRUE; }

public:
	virtual ~CPlaylistParseJsonThread() = default;

public:
	BOOL InitInstance() override;

	void SetData(ThreadConfig& config) { m_config = std::move(config); };
	void SetPlugin(std::shared_ptr<base_plugin>& parent_plugin) { m_parent_plugin = parent_plugin; };

protected:
	void ParseSharaclub();
	void ParseCbilling();
	void ParseEdem();
	void ParseGlanz();

protected:
	ThreadConfig m_config;
	std::shared_ptr<base_plugin> m_parent_plugin;
};
