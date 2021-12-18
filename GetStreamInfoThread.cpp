// GetStreamInfoThread.cpp : implementation file
//

#include "pch.h"
#include <thread>
#include "GetStreamInfoThread.h"
#include "map_serializer.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CGetStreamInfoThread::ThreadConfig::NotifyParent(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
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
		const auto max_threads = m_config.m_max_threads;

		m_config.NotifyParent(WM_UPDATE_PROGRESS_STREAM, 0, m_config.m_container->size());

		auto newEnd = std::unique(m_config.m_container->begin(), m_config.m_container->end());
		for (auto it = m_config.m_container->begin(); it != newEnd;)
		{
			if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;

			std::vector<std::thread> workers(max_threads);
			std::vector<std::string> audio(max_threads);
			std::vector<std::string> video(max_threads);
			auto pool = it;
			int j = 0;
			while (j < max_threads && pool != m_config.m_container->end())
			{
				const auto& url = (*pool)->get_templated(m_config.m_StreamSubtype, m_config.m_params);
				workers[j] = std::thread(GetChannelStreamInfo, m_config.m_probe, url, std::ref(audio[j]), std::ref(video[j]));
				j++;
				++pool;
			}

			j = 0;
			for (auto& w : workers)
			{
				if (!w.joinable()) continue;

				w.join();

				auto hash = (*it)->get_hash();
				std::pair<std::string, std::string> info(audio[j], video[j]);
				auto& pair = stream_infos->emplace(hash, info);
				if (!pair.second)
				{
					pair.first->second = std::move(info);
				}

				++it;
				j++;

				auto step = std::distance(m_config.m_container->begin(), it);
				m_config.NotifyParent(WM_UPDATE_PROGRESS_STREAM, step, m_config.m_container->size());
			}
		}
	}

	m_config.NotifyParent(WM_END_GET_STREAM_INFO, (WPARAM)stream_infos.release());

	CoUninitialize();

	return FALSE;
}

void CGetStreamInfoThread::GetChannelStreamInfo(const std::wstring& probe, const std::wstring& url, std::string& audio, std::string& video)
{
	if (url.empty())
		return;

	SECURITY_ATTRIBUTES sa;
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
	csCommand.Format(_T("\"%s\" -hide_banner -show_streams \"%s\""), probe.c_str(), url.c_str());

	BOOL bRunProcess = CreateProcess(probe.c_str(),				// 	__in_opt     LPCTSTR lpApplicationName
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
				TRACE("Success! Exit code: %u for %s\n", dwExitCode, url.c_str());
				break;
			}

			if (utils::CheckForTimeOut(dwStart, nTimeout * 1000))
			{
				bTimeout = TRUE;
				::TerminateProcess(pi.hProcess, 0);
				TRACE("Failed! Execution Timeout. %s\n", url.c_str());
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
				TRACE("GetExitCodeProcess failed. ErrorCode: %0u, try count: %0d, source %hs\n", ::GetLastError(), nErrorCount, url.c_str());
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

	audio = "Not available";
	video = "Not available";
	for (auto& stream : streams)
	{
		if (stream["codec_type"] == "audio")
		{
			audio = stream["codec_long_name"] + " ";
			audio += stream["sample_rate"] + " ";
			audio += stream["channel_layout"];
		}
		else if (stream["codec_type"] == "video")
		{
			video = stream["width"] + "x";
			video += stream["height"] + " ";
			video += stream["codec_long_name"];
		}
	}
}
