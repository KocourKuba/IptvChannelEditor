/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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

class CBaseThread : public CWinThread
{
public:
	class ThreadConfig
	{
	public:
		void SendNotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
		void PostNotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

		std::stringstream m_data;
		void* m_parent = nullptr;
		HANDLE m_hStop = nullptr;
		HANDLE m_hExit = nullptr;
		std::wstring m_rootPath;
		std::wstring m_url;
		std::string nparam;
		std::wstring wparam;
		int m_cache_ttl = 0;
	};

protected:
	CBaseThread() { m_bAutoDelete = TRUE; }

public:
	virtual ~CBaseThread() = default;

	void SetData(ThreadConfig& config) { m_config = std::move(config); };
	void SetPlugin(std::shared_ptr<base_plugin>& parent_plugin) { m_parent_plugin = parent_plugin; };

protected:
	ThreadConfig m_config;
	std::shared_ptr<base_plugin> m_parent_plugin{};
};
