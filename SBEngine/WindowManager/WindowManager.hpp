#pragma once

#include <Windows.h>
#include <string>
#include <DirectXMath.h>
#include <d3d11_4.h>
#include "../Direct3DResources/Direct3DResources.hpp"

class WindowManager_t {
private:
public:
    inline static DirectX::XMFLOAT2 WindowSize = { 0.0f, 0.0f };
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