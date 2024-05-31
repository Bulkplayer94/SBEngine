#pragma once

#include <string>
#include <ctime>

class Logger_t {
public:

	enum LogType : uint8_t {
		LogType_SYSTEM = 14,
		LogType_DEFAULT = 7,
		LogType_ERROR = 12,
	};

private:

	// Last time logger printed
	time_t LastPrint = 0; 

	// Formatted time to string
	std::string Time = ""; 

	// Current logtype
	LogType CurrentLogType = LogType_DEFAULT; 

public:

	void ConsoleLog(const std::string& Text, LogType Type);
	void HandleError(int Line, const char* File);

} extern Logger;