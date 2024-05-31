#include "WindowManager.hpp"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_dx11.h"
#include "../ImGui/imgui_impl_win32.h"
#include <iostream>

WindowManager_t WindowManager;

bool WindowManager_t::Init(const std::string& WindowName, const DirectX::XMFLOAT2& WindowSize) {
	// Center the window
	DirectX::XMFLOAT2 WindowPos = { 0.0f, 0.0f };

	const int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	WindowPos.x = ScreenWidth / 2 - WindowSize.x / 2;
	WindowPos.y = ScreenWidth / 2 - WindowSize.y / 2;


	// Create windowclass and window
	const WNDCLASSA WindowClass = { CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, WindowName.c_str() };
	ATOM ret = RegisterClassA(&WindowClass);
	if (ret == 0) {
		std::cout << "Failed to create windowclass " << std::hex << GetLastError() << std::endl;
		return false;
	}

	this->WindowHandle = CreateWindowA(WindowName.c_str(), WindowName.c_str(), WS_OVERLAPPEDWINDOW, static_cast<UINT>(WindowPos.x), static_cast<UINT>(WindowPos.y), static_cast<UINT>(WindowSize.x), static_cast<UINT>(WindowSize.y), NULL, NULL, WindowClass.hInstance, NULL);
	if (this->WindowHandle == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to create window " << std::hex << GetLastError() << std::endl;
		return false;
	}


	// Show window
	ShowWindow(this->WindowHandle, SW_SHOWDEFAULT);
	UpdateWindow(this->WindowHandle);
	
	this->IsRunning = true;

	return true;
}

bool WindowManager_t::HookupImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	if (!ImGui_ImplWin32_Init(this->WindowHandle)) {
		std::cout << "'ImGui_ImplWin32_Init' failed " << std::hex << GetLastError() << std::endl;
		this->IsRunning = false;
		return false;
	}
	if (!ImGui_ImplDX11_Init(SBEngine::Direct3DResources.d3d11Device, SBEngine::Direct3DResources.d3d11DeviceContext)) {
		std::cout << "'ImGui_ImplDX11_Init' failed " << std::hex << GetLastError() << std::endl;
		this->IsRunning = false;
		return false;
	}

	this->IsRunning = true;
}

void WindowManager_t::Begin() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	SBEngine::Direct3DResources.d3d11DeviceContext->ClearRenderTargetView(SBEngine::Direct3DResources.d3d11RenderTargetView, CanvasColor);
}

void WindowManager_t::End() {
	ImGui::Render();

	ID3D11RenderTargetView* RenderTargetView = SBEngine::Direct3DResources.d3d11RenderTargetView;
	SBEngine::Direct3DResources.d3d11DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	SBEngine::Direct3DResources.dxgiSwapChain->Present(this->VSync, 0);

	MSG Message;
	while (::PeekMessage(&Message, nullptr, 0U, 0U, PM_REMOVE)) {
		::TranslateMessage(&Message);
		::DispatchMessage(&Message);

		switch (Message.message) {
			case WM_QUIT: {
				SBEngine::Direct3DResources.Release();
				this->IsRunning = false;
				break;
			}
			case WM_SIZE: {
				SBEngine::Direct3DResources.Resize();
			}
		}
	}
}