#pragma once
#include <d3d11_4.h>

namespace SBEngine {
	struct Direct3DResources_t {
		ID3D11Device5* d3d11Device = nullptr;
		ID3D11DeviceContext4* d3d11DeviceContext = nullptr;
		IDXGISwapChain4* dxgiSwapChain = nullptr;

		ID3D11RasterizerState2* d3d11RasterizerState = nullptr;
		ID3D11SamplerState* d3d11SamplerState = nullptr;
		ID3D11DepthStencilState* d3d11DepthStencilState = nullptr;

		ID3D11DepthStencilView* d3d11DepthStencilView = nullptr;
		ID3D11RenderTargetView1* d3d11RenderTargetView = nullptr;

		bool Init();
		bool Resize();
		bool Release();
	} extern Direct3DResources;
}