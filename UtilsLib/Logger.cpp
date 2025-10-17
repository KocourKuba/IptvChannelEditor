#include "pch.h"
#include <iostream>
#include <atltrace.h>
#include "Logger.h"
#include "utils.h"

namespace utils
{

Logger::Logger() : m_isRunning(true), m_workerThread(&Logger::processLogs, this)
{
}

Logger::~Logger()
{
	stop();
	closeLog();
}

void Logger::log(const std::string& message)
{
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_logQueue.push(message);
		ATLTRACE("%s\n", message.c_str());
	}
	m_cv.notify_one(); // Notify the worker thread
}

void Logger::log(const std::wstring& message)
{
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_logQueue.push(utils::utf16_to_utf8(message));
		ATLTRACE(L"%s\n", message.c_str());
	}
	m_cv.notify_one(); // Notify the worker thread
}

void Logger::setLogName(const std::wstring& name)
{
	closeLog();
	m_logFilename = name;
	openLog();
}

void Logger::stop()
{
	m_isRunning = false;
	m_cv.notify_one(); // Wake up the worker thread to finish
	if (m_workerThread.joinable())
	{
		m_workerThread.join();
	}
}

void Logger::openLog()
{
	if (!m_logFilename.empty())
	{
		m_logFile.open(m_logFilename, std::ios::binary | std::ios::out | std::ios::app);
	}
}

void Logger::closeLog()
{
	if (m_logFile.is_open())
	{
		m_logFile.close();
	}
}

void Logger::processLogs()
{
	while (m_isRunning || !m_logQueue.empty())
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_cv.wait(lock, [this]()
				  {
					  return !m_isRunning || !m_logQueue.empty();
				  });


		while (!m_logQueue.empty())
		{
			if (m_logFile.is_open())
			{
				auto now = std::chrono::system_clock::now();
				// Convert the time_point to a time_t for formatting
				auto time_t_now = std::chrono::system_clock::to_time_t(now);

				// Extract the milliseconds part
				auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()) % 60;
				auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

				// Format the time_point with date and milliseconds
				std::string formatted_time = std::format("[{:%F %H:%M}:{:02}.{:03}]",
														 std::chrono::system_clock::from_time_t(time_t_now),
														 seconds.count(),
														 milliseconds.count());

				std::string line;
				std::stringstream ss(m_logQueue.front());
				while (std::getline(ss, line))
				{
					while (!line.empty() && line.ends_with('\r'))
					{
						line.pop_back();
					}

					if (!line.empty())
					{
						m_logFile << formatted_time << ' ' << line << std::endl;
					}
				}

			}

			m_logQueue.pop();
		}
	}
}

}
