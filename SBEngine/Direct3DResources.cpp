#include "Direct3DResources.hpp"
#include <iostream>

using namespace SBEngine;

Direct3DResources_t SBEngine::Direct3DResources;

bool Direct3DResources_t::Init(HWND hWnd)
{
	{
		ID3D11Device* baseDevice;
		ID3D11DeviceContext* baseDeviceContext;
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1 };
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

		HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
			0, creationFlags,
			featureLevels, ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, &baseDevice,
			0, &baseDeviceContext);

		if (FAILED(hResult)) {
			std::cout << "D3D11 BaseDevice Creation Failed!" << std::endl;
			return false;
		}

		hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device5), (void**)&d3d11Device);
		if (FAILED(hResult)) {
			std::cout << "D3D11.5 Device Creation Failed!" << std::endl;
			return false;
		}
		baseDevice->Release();

		hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext4), (void**)&d3d11DeviceContext);
		if (FAILED(hResult)) {
			std::cout << "D3D11 Device Context Creation Failed!" << std::endl;
			return false;
		}
		baseDeviceContext->Release();
	}

#ifdef _DEBUG

	{
		ID3D11Debug* d3d11Debug;
		d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3d11Debug);
		if (d3d11Debug) {
			ID3D11InfoQueue* d3d11InfoQueue = nullptr;
			d3d11Debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)d3d11InfoQueue);
			if (d3d11InfoQueue) {
				d3d11InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3d11InfoQueue->GetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR);
				d3d11InfoQueue->Release();
			}
			d3d11Debug->Release();
		}
	}

#endif // _DEBUG

	{
		IDXGIFactory5* dxgiFactory;
		{
			IDXGIDevice4* dxgiDevice;
			HRESULT hResult = d3d11Device->QueryInterface(__uuidof(IDXGIDevice4), (void**)&dxgiDevice);
			if (FAILED(hResult)) {
				std::cout << "DXGIDevice5 Creation Failed!" << std::endl;
				return false;
			}

			IDXGIAdapter3* dxgiAdapter;
			hResult = dxgiDevice->QueryInterface(__uuidof(IDXGIAdapter3), (void**)&dxgiAdapter);
			if (FAILED(hResult)) {
				std::cout << "IDXGIAdapter3 Creation Failed!" << std::endl;
				return false;
			}

			DXGI_ADAPTER_DESC2 adapterDesc;
			dxgiAdapter->GetDesc2(&adapterDesc);
			std::wcout << L"Graphics Device: " << adapterDesc.Description << std::endl;

			hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory5), (void**)&dxgiFactory);
			if (FAILED(hResult)) {
				std::cout << "IDXGIFactory5 Creation Failed!" << std::endl;
				return false;
			}

			dxgiAdapter->Release();
		}

		DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
		d3d11SwapChainDesc.Width = 0;
		d3d11SwapChainDesc.Height = 0;
		d3d11SwapChainDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		d3d11SwapChainDesc.SampleDesc.Count = 1;
		d3d11SwapChainDesc.SampleDesc.Quality = 0;
		d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		d3d11SwapChainDesc.BufferCount = 2;
		d3d11SwapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
		d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		d3d11SwapChainDesc.Flags = 0;

		IDXGISwapChain1* interpretedSwapChain = reinterpret_cast<IDXGISwapChain1*>(dxgiSwapChain);
		HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(d3d11Device, hWnd, &d3d11SwapChainDesc, NULL, NULL, &interpretedSwapChain);
		if (FAILED(hResult)) {
			std::cout << "Swap Chain Creation Failed!" << std::endl;
			return false;
		}
	}

	Resize();

	{
		D3D11_RASTERIZER_DESC2 rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID; // Fülle die Dreiecke
		rasterizerDesc.CullMode = D3D11_CULL_BACK; // Aktiviere Backface Culling
		rasterizerDesc.FrontCounterClockwise = TRUE; // Die Dreiecksnormalen zeigen im Uhrzeigersinn (standardmäßig)
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;

		HRESULT hResult = d3d11Device->CreateRasterizerState2(&rasterizerDesc, &d3d11RasterizerState);
		if (FAILED(hResult)) {
			std::cout << "Rasterizer State Creation Failed!" << std::endl;
			return false;
		}
	}

	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		HRESULT hResult = d3d11Device->CreateDepthStencilState(&depthStencilDesc, &d3d11DepthStencilState);
		if (FAILED(hResult)) {
			std::cout << "Depth Stencil State Creation Failed!" << std::endl;
			return false;
		}
	}

	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0; // Setze MinLOD und MaxLOD auf ihre Standardwerte
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT hResult = d3d11Device->CreateSamplerState(&samplerDesc, &d3d11SamplerState);
		if (FAILED(hResult)) {
			std::cout << "Sampler State Creation Failed!" << std::endl;
			return false;
		}
	}

	return true;
}

bool Direct3DResources_t::Resize(bool firstTime)
{
	if (!firstTime) {
		d3d11DeviceContext->OMSetRenderTargets(0, 0, 0);
		d3d11RenderTargetView->Release();
		d3d11DepthStencilView->Release();

		HRESULT hResult = dxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(hResult))
			return false;

		//perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(84), 0.1f, 1000.f);
	}

	ID3D11Texture2D* d3d11FrameBuffer;
	HRESULT hResult = dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture3D), (void**)&d3d11FrameBuffer);
	if (FAILED(hResult))
		goto FAILURE_END;

	hResult = d3d11Device->CreateRenderTargetView1(d3d11FrameBuffer, 0, &d3d11RenderTargetView);
	if (FAILED(hResult))
		goto FAILURE_END;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	d3d11FrameBuffer->GetDesc(&depthBufferDesc);

	d3d11FrameBuffer->Release();

	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthBuffer;
	d3d11Device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

	d3d11Device->CreateDepthStencilView(depthBuffer, nullptr, &d3d11DepthStencilView);

	depthBuffer->Release();

	return true;

FAILURE_END:

	std::cout << "Buffer Resizing Failed!" << std::endl;
	return false;
}

bool Direct3DResources_t::Release()
{
	d3d11Device->Release();
	d3d11DeviceContext->Release();
	dxgiSwapChain->Release();

	d3d11RasterizerState->Release();
	d3d11SamplerState->Release();
	d3d11DepthStencilState->Release();

	d3d11DepthStencilView->Release();
	d3d11RenderTargetView->Release();

	return true;
}
