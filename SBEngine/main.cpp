#include "Logger/Logger.hpp"
#include "WindowManager/WindowManager.hpp"


int main() {
	WindowManager.Init("Test", { 800.0f, 600.0f });
	//
	//while (true) {
	//
	//}

	Logger.ConsoleLog("Test", Logger_t::LogType_ERROR);
	Logger.ConsoleLog("Test", Logger_t::LogType_DEFAULT);
	Logger.ConsoleLog("Test", Logger_t::LogType_SYSTEM);

	return 0;
}