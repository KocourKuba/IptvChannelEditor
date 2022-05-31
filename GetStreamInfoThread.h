/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
	};

protected:
	CGetStreamInfoThread() { m_bAutoDelete = TRUE; }

public:
	virtual ~CGetStreamInfoThread() { delete m_config.m_container; }

public:
	BOOL InitInstance() override;

	void SetData(const ThreadConfig& config) { m_config = config; };

protected:
	static void GetChannelStreamInfo(ThreadConfig& config, std::atomic<int>& count, int index);

protected:
	ThreadConfig m_config;
	std::wregex m_re;
};
