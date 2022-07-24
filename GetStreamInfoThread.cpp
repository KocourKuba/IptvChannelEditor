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

#include "pch.h"
#include <thread>
#include "GetStreamInfoThread.h"
#include "map_serializer.h"

#include "UtilsLib\utils.h"
#include "thread-pool\BS_thread_pool.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CGetStreamInfoThread::ThreadConfig::NotifyParent(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) const
{
	if (m_parent->GetSafeHwnd())
		m_parent->SendMessage(message, wParam, lParam);
}

IMPLEMENT_DYNCREATE(CGetStreamInfoThread, CWinThread)

BOOL CGetStreamInfoThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	auto stream_infos = std::make_unique<serializable_map>();

	if (m_config.m_container)
	{
		m_config.NotifyParent(WM_INIT_PROGRESS, (WPARAM)m_config.m_container->size(), 0);

		ULARGE_INTEGER ul = { 0, (DWORD)m_config.m_container->size() };
		m_config.NotifyParent(WM_UPDATE_PROGRESS_STREAM, (WPARAM)&ul);

		BS::thread_pool pool(m_config.m_max_threads);
		std::atomic<int> count { 0 };
		const auto& res = pool.parallelize_loop(0, (int)m_config.m_container->size(), [this, &count](const auto& a, const auto& b)
												{
													for (auto i = a; i < b; i++)
													{
														GetChannelStreamInfo(m_config, count, i);
													}
												});
	}

	m_config.NotifyParent(WM_END_GET_STREAM_INFO);

	CoUninitialize();

	return FALSE;
}

void CGetStreamInfoThread::GetChannelStreamInfo(ThreadConfig& config, std::atomic<int>& count, int index)
{
	TRACE("GetChannelStreamInfo: thread %d start\n", index);
	auto uri = config.m_container->at(index);
	const auto& url = uri->get_templated_stream(config.m_params);

	if (url.empty() || ::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0)
		return;

	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;

	// Create a pipe for the child process's STDOUT
	HANDLE hStdoutRd;
	HANDLE hChildStdoutWr = nullptr;
	HANDLE hChildStdoutRd = nullptr;
	CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &sa, 0);

	// Duplicate the read handle to the pipe so it is not inherited and close src handle
	HANDLE hSelf = GetCurrentProcess();
	if (!DuplicateHandle(hSelf, hChildStdoutRd, hSelf, &hStdoutRd, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	{
		TRACE(_T("Failed! Can't create stdout pipe to child process. Code: %u\n"), GetLastError());
		return;
	}

	std::string result;
	BOOL bResult = FALSE;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.hStdInput = nullptr;
	si.hStdOutput = hChildStdoutWr;

	PROCESS_INFORMATION pi = { nullptr };

	// argv[0] имя исполняемого файла
	CString csCommand;
	csCommand.Format(_T("\"%s\" -hide_banner -show_streams \"%s\""), config.m_probe.c_str(), url.c_str());

	BOOL bRunProcess = CreateProcess(config.m_probe.c_str(),	// 	__in_opt     LPCTSTR lpApplicationName
									 csCommand.GetBuffer(0),	// 	__inout_opt  LPTSTR lpCommandLine
									 nullptr,					// 	__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes
									 nullptr,					// 	__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes
									 TRUE,						// 	__in         BOOL bInheritHandles
									 CREATE_SUSPENDED,			// 	__in         DWORD dwCreationFlags
									 nullptr,					// 	__in_opt     LPVOID lpEnvironment
									 nullptr,					// 	__in_opt     LPCTSTR lpCurrentDirectory
									 &si,						// 	__in         LPSTARTUPINFO lpStartupInfo
									 &pi);						// 	__out        LPPROCESS_INFORMATION lpProcessInformation

	if (!bRunProcess)
	{
		TRACE(_T("Failed! Can't execute command: %s\nCode: %u\n"), csCommand.GetString(), GetLastError());
	}
	else
	{
		::ResumeThread(pi.hThread);

		long nTimeout = 60;

		int nErrorCount = 0;
		DWORD dwExitCode = STILL_ACTIVE;
		uint64_t dwStart = utils::ChronoGetTickCount();
		BOOL bTimeout = FALSE;
		for (;;)
		{
			if (dwExitCode != STILL_ACTIVE)
			{
				TRACE("Success! Exit code: %u for %ls\n", dwExitCode, url.c_str());
				break;
			}

			if (utils::CheckForTimeOut(dwStart, nTimeout * 1000))
			{
				bTimeout = TRUE;
				::TerminateProcess(pi.hProcess, 0);
				TRACE("Failed! Execution Timeout. %ls\n", url.c_str());
				break;
			}

			if (::GetExitCodeProcess(pi.hProcess, &dwExitCode))
			{
				for (;;)
				{
					if (utils::CheckForTimeOut(dwStart, nTimeout * 1000)) break;

					// Peek data from stdout pipe
					DWORD dwAvail = 0;
					if (!::PeekNamedPipe(hStdoutRd, nullptr, 0, nullptr, &dwAvail, nullptr) || !dwAvail) break;

					char szBuf[4096] = { 0 };
					DWORD dwReaded = dwAvail;
					if (!ReadFile(hStdoutRd, szBuf, min(4095, dwAvail), &dwReaded, nullptr)) break;

					result.append(szBuf, dwReaded);
				}
			}
			else
			{
				// По каким то причинам обломался
				TRACE("GetExitCodeProcess failed. ErrorCode: %0u, try count: %0d, source %ls\n", ::GetLastError(), nErrorCount, url.c_str());
				nErrorCount++;
				if (nErrorCount > 10) break;
				continue;
			}
		}

		bResult = TRUE;
	}

	::CloseHandle(hStdoutRd);
	::CloseHandle(hChildStdoutWr);
	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);

	// Parse stdout
	std::stringstream ss(result);
	std::string item;
	std::vector<std::map<std::string, std::string>> streams;
	bool skip = false;
	size_t idx = 0;
	while (std::getline(ss, item))
	{
		utils::string_rtrim(item, "\r\n");
		if (item == "[STREAM]")
		{
			skip = false;
			streams.emplace_back(std::map<std::string, std::string>());
			idx = streams.size() - 1;
			continue;
		}

		if (item == "[/STREAM]")
		{
			skip = true;
			continue;
		}

		if (!skip)
		{
			auto pair = utils::string_split(item, '=');
			if (pair.size() > 1)
				streams[idx].emplace(pair[0], pair[1]);
		}
	}

	int a = 1;
	std::string audio;
	std::string video;

	for (auto& stream : streams)
	{
		if (stream["codec_type"] == "audio")
		{
			audio += fmt::format("#{:d} {:s}, {:s}, {:s} ", a++, stream["codec_long_name"], stream["sample_rate"], stream["channel_layout"]);
		}
		else if (stream["codec_type"] == "video")
		{
			video = fmt::format("{:s}x{:s}", stream["width"], stream["height"]);
			double fps = 0.0F;
			try
			{
				auto pos = stream["r_frame_rate"].find('/');
				auto fps1 = std::stoi(stream["r_frame_rate"].substr(0, pos));
				pos = (pos != std::string::npos) ? ++pos : pos;
				auto fps2 = std::stoi(stream["r_frame_rate"].substr(pos));
				fps = (double)fps1 / (double)fps2;
				double integer;
				double fractional = modf(fps, &integer);
				if (fractional > 0)
					video += fmt::format(", {:.3f}fps", fps);
				else
					video += fmt::format(", {:d}fps", (int)integer);
			}
			catch (...)
			{
			}

			video += fmt::format(", {:s}", stream["codec_long_name"]);
		}
	}

	if (audio.empty())
		audio = "Not available";

	if (video.empty())
		video = "Not available";

	TRACE("GetChannelStreamInfo: Thread %d, Video: %s, Audio: %s\n", (int)count, video.c_str(), audio.c_str());

	ULARGE_INTEGER ul = { (DWORD)++count, (DWORD)config.m_container->size() };
	std::tuple<int, std::string, std::string> tuple = { uri->get_hash(), audio, video };

	config.NotifyParent(WM_UPDATE_PROGRESS_STREAM, (WPARAM)&ul, (LPARAM)&tuple);
}
