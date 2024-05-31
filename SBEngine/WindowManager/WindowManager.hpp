#pragma once

#include <Windows.h>
#include <string>
#include <DirectXMath.h>
#include <d3d11_4.h>
#include "../Direct3DResources.hpp"

static LRESULT WINAPI WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    switch (Msg) {
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

class WindowManager_t {
private:

public:
    HWND WindowHandle = NULL;
    float AspectRatio = 0.0f;
    bool IsRunning = false;
    float CanvasColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    bool VSync = false;

    // resize event
    // not running

    bool Init(const std::string& WindowName, const DirectX::XMFLOAT2& WindowSize);
    bool HookupImGui();

    void Begin();
    void End();
} extern WindowManager;