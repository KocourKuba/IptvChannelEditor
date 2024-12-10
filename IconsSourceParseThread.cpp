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

#include "pch.h"
#include "IconsSourceParseThread.h"
#include "IconCache.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CIconsSourceParseThread, CWinThread)

BOOL CIconsSourceParseThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	auto entries = std::make_unique<std::vector<CIconSourceData>>();

	if (m_config.m_data)
	{
		const auto& wbuf = m_config.m_data.str();
		boost::regex re(m_config.nparam);
		std::istringstream stream(wbuf);
		if (stream.good())
		{
			m_config.SendNotifyParent(WM_INIT_PROGRESS, (int)std::count(wbuf.begin(), wbuf.end(), '\n'), 0);

			int num = 0;
			int step = 0;

			std::string line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, "\r");
				step++;

				boost::smatch m;
				if (!boost::regex_match(line, m, re))
					continue;

				CIconSourceData entry;
				entry.logo_path = utils::utf8_to_utf16(utils::string_replace<char>(m[1].str(), "https://", "http://"));
				entry.logo_name = utils::utf8_to_utf16(m[2].str());
				entry.logo_id = utils::utf8_to_utf16(m[3].str());

				entries->emplace_back(entry);
				num++;
				if (num % 100 == 0)
				{
					m_config.SendNotifyParent(WM_UPDATE_PROGRESS, num, step);
					if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
					{
						entries.release();
						break;
					}
				}
			}
		}
	}

	m_config.SendNotifyParent(WM_END_LOAD_PLAYLIST, (WPARAM)entries.release());

	::SetEvent(m_config.m_hExit);
	ATLTRACE("\nThread exit\n");

	CoUninitialize();

	return FALSE;
}
