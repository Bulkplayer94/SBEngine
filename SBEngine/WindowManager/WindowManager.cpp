#include "WindowManager.hpp"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_dx11.h"
#include "../ImGui/imgui_impl_win32.h"
#include <iostream>
#include "../Logger/Logger.hpp"

WindowManager_t WindowManager;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
		return 0;
	
	switch (Msg) {
	case WM_SIZE:
		SBEngine::Direct3DResources.Resize();
		WindowManager_t::WindowSize = DirectX::XMFLOAT2(static_cast<float>(LOWORD(wParam)), static_cast<float>(HIWORD(lParam)));
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, Msg, wParam, lParam);
}

bool WindowManager_t::Init(const std::string& WindowName, const DirectX::XMFLOAT2& WindowSize) {

	this->WindowSize = WindowSize;

	// Center the window
	DirectX::XMFLOAT2 WindowPos = { 0.0f, 0.0f };

	const int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	WindowPos.x = ScreenWidth / 2 - WindowSize.x / 2;
	WindowPos.y = ScreenHeight / 2 - WindowSize.y / 2;

	std::wstring WWindowName(WindowName.begin(), WindowName.end());

	const WNDCLASSEXW WindowClass = { 
		sizeof(WindowClass), 
		CS_CLASSDC, 
		WndProc, 
		0L, 
		0L, 
		GetModuleHandle(nullptr), 
		nullptr, 
		nullptr, 
		nullptr, 
		nullptr, 
		WWindowName.c_str(),
		nullptr
	};
	if (!RegisterClassExW(&WindowClass)) {
		Logger.HandleError(__LINE__, __FILE__);
		return false;
	}

	this->WindowHandle = CreateWindowW(
		WWindowName.c_str(), 
		WWindowName.c_str(),
		WS_OVERLAPPEDWINDOW, 
		static_cast<int>(WindowPos.x),
		static_cast<int>(WindowPos.y),
		static_cast<int>(WindowSize.x), 
		static_cast<int>(WindowSize.y), 
		NULL, 
		NULL, 
		WindowClass.hInstance, 
		NULL
	);

	if (this->WindowHandle == INVALID_HANDLE_VALUE || this->WindowHandle == NULL) {
		Logger.HandleError(__LINE__, __FILE__);
		return false;
	}

	this->IsRunning = true;

	return true;
}

bool WindowManager_t::HookupImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	if (!ImGui_ImplWin32_Init(this->WindowHandle)) {
		Logger.HandleError(__LINE__, __FILE__);
		this->IsRunning = false;
		return false;
	}
	if (!ImGui_ImplDX11_Init(SBEngine::Direct3DResources.d3d11Device, SBEngine::Direct3DResources.d3d11DeviceContext)) {
		Logger.HandleError(__LINE__, __FILE__);
		this->IsRunning = false;
		return false;
	}

	// Show window
	ShowWindow(this->WindowHandle, SW_SHOWDEFAULT);
	UpdateWindow(this->WindowHandle);

	this->IsRunning = true;
	return true;
}

void WindowManager_t::Begin() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	SBEngine::Direct3DResources.d3d11DeviceContext->ClearRenderTargetView(SBEngine::Direct3DResources.d3d11RenderTargetView, CanvasColor);
	SBEngine::Direct3DResources.d3d11DeviceContext->ClearDepthStencilView(SBEngine::Direct3DResources.d3d11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	D3D11_VIEWPORT ViewPort = { 0.0f, 0.0f, static_cast<FLOAT>(this->WindowSize.x), static_cast<FLOAT>(this->WindowSize.y) };
	SBEngine::Direct3DResources.d3d11DeviceContext->RSSetViewports(1, &ViewPort);

}

void WindowManager_t::End() {
	ImGui::Render();

	ID3D11RenderTargetView* RenderTargetView = SBEngine::Direct3DResources.d3d11RenderTargetView;
	SBEngine::Direct3DResources.d3d11DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	SBEngine::Direct3DResources.dxgiSwapChain->Present(this->VSync, 0);

	MSG Message;
	while (::PeekMessage(&Message, nullptr, 0U, 0U, PM_REMOVE)) {
		switch (Message.message) {
			case WM_QUIT: {
				SBEngine::Direct3DResources.Release();
				this->IsRunning = false;
				break;
			}
		}

		::TranslateMessage(&Message);
		::DispatchMessage(&Message);
	}
}