#pragma once
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace utils
{

class Logger
{
public:
	// Get the singleton instance of the Logger
	static Logger& getInstance()
	{
		static Logger instance;
		return instance;
	}

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	// Log a message (thread-safe)
	void log(const std::string& message);
	void log(const std::wstring& message);

	// set log file name
	void setLogName(const std::wstring& name);

	// Stop the logger and flush remaining messages
	void stop();

private:
	Logger();
	// Destructor
	~Logger();

	// Worker thread function to process log messages
	void processLogs();
	void openLog();
	void closeLog();

	std::wstring m_logFilename;				// Worker thread for asynchronous logging
	std::ofstream m_logFile;                // Log file stream
	std::queue<std::string> m_logQueue{};   // Queue to store log messages
	std::mutex m_queueMutex;                // Mutex to protect the queue
	std::condition_variable m_cv;			// Condition variable for thread synchronization
	std::atomic<bool> m_isRunning;          // Atomic flag to control the logger
	std::thread m_workerThread;             // Worker thread for asynchronous logging

};

}

#define LOG_PROTOCOL utils::Logger::getInstance().log