#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>
#include <string>

class ShaderManager_t {
private:
	ID3D11InputLayout** InputLayout;
public:

	typedef void(*SHADER_UPDATE_BUFFER)(ID3D11DeviceContext*, DirectX::XMFLOAT4X4);
	typedef void(*SHADER_SET_CONTEXT)(ID3D11DeviceContext*, const std::string&);

	struct SHADER {
		std::string ShaderName;

		ID3D11VertexShader** VSShader;
		ID3D11PixelShader** PSShader;
		ID3D11Buffer** ConstantBuffer;

		SHADER_UPDATE_BUFFER sBufferFunc;
		SHADER_SET_CONTEXT sContextFunc;
	};

	bool SetContext(const std::string& ShaderName);

	bool StartCompiling();

} extern ShaderManager;