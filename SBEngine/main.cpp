#include "Logger/Logger.hpp"
#include "WindowManager/WindowManager.hpp"
#include "Direct3DResources/Direct3DResources.hpp"
#include "ShaderManager/ShaderManager.hpp"
#include "MeshManager/MeshManager.hpp"
#include "PhysXManager/PhysXManager.hpp"
#include "Camera/Camera.hpp"
#include "ImGui/imgui.h"


int main() {
	WindowManager.Init("Test", { 800.0f, 600.0f });
	SBEngine::Direct3DResources.Init(WindowManager.WindowHandle);

	WindowManager.HookupImGui();

	ShaderManager.StartCompiling();
	SBEngine::PhysXManager.SetupPhysX();
	SBEngine::meshManager.StartLoading();

	while (WindowManager.IsRunning) {
		WindowManager.Begin();
		SBEngine::Camera.Proc(0.016F);

		ImGui::Begin("Hund");
		ImGui::Text("Richtiger Hund");
		ImGui::End();

		WindowManager.End();
	}

	Logger.ConsoleLog("Test", Logger_t::LogType_ERROR);
	Logger.ConsoleLog("Test", Logger_t::LogType_DEFAULT);
	Logger.ConsoleLog("Test", Logger_t::LogType_SYSTEM);

	return 0;
}