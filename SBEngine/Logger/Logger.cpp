#include "Logger.hpp"
#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>

Logger_t Logger;

void Logger_t::ConsoleLog(const std::string& Text, LogType Type) {
	
	// Incase the console handle goes invalid
	static HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!ConsoleHandle)
		ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get the time
	time_t CurTime = time(nullptr);

	// Only execute 'std::strftime' when a second has passed
	if ((CurTime - LastPrint) > 0) {

		// Set time
		LastPrint = CurTime;

		// Convert time_t to std::tm
		std::tm LocalTime;
		localtime_s(&LocalTime, const_cast<time_t*>(&CurTime));

		// Format string
		char Buffer[9];
		std::strftime(Buffer, sizeof(Buffer), "%H:%M:%S", &LocalTime);

		// Save formatted string
		this->Time = std::string(Buffer);
	}
	
	// Only call 'SetConsoleTextAttribute' when the type changes
	if (this->CurrentLogType != Type) {
		SetConsoleTextAttribute(ConsoleHandle, Type);
		this->CurrentLogType = Type;
	}

	std::cout << Time;

	switch (Type) {
		case LogType_SYSTEM: {
			
			std::cout << " [SYSTEM] ";

			break;
		}
		case LogType_DEFAULT: {

			std::cout << " [DEFAULT] ";

			break;
		}
		case LogType_ERROR: {

			std::cout << " [ERROR] ";

			break;
		}
	}

	std::cout << Text << "\n";
}

void Logger_t::HandleError(int Line, const char* File) {
	DWORD ErrorCode = GetLastError();
	if (ErrorCode == 0)
		return; // No error occured

	LPSTR MessageBuffer = nullptr;

	// Get a message from ErrorCode
	size_t Size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&MessageBuffer),
		0,
		NULL
	);

	std::ostringstream oss;
	oss << "File: '" << File << "' Line: " << Line << " " << std::string(MessageBuffer, Size);

	this->ConsoleLog(oss.str(), LogType_ERROR);
}