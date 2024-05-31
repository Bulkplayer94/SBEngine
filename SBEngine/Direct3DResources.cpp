#include "Direct3DResources.hpp"

using namespace SBEngine;

Direct3DResources_t SBEngine::Direct3DResources;

bool Direct3DResources_t::Init()
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

	return true;
}

bool Direct3DResources_t::Resize()
{
	return true;
}
