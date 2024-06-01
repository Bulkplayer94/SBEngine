#include "ShaderManager.hpp"
#include "../Direct3DResources/Direct3DResources.hpp"
#include "../Logger/Logger.hpp"
#include <filesystem>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

ShaderManager_t ShaderManager;

bool ShaderManager_t::SetContext(const std::string& ShaderName) {
	return true;
}

bool ShaderManager_t::StartCompiling() {

	{
		D3D11_BUFFER_DESC commonDesc = {};
		commonDesc.ByteWidth = sizeof(CommonCBuffer_t) + 0xf & 0xfffffff0;
		commonDesc.Usage = D3D11_USAGE_DYNAMIC;
		commonDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		commonDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = SBEngine::Direct3DResources.d3d11Device->CreateBuffer(&commonDesc, nullptr, &commonCBuffer);
		if (FAILED(hResult)) {
			Logger.HandleError(__LINE__, __FILE__);
			return false;
		}

		SBEngine::Direct3DResources.d3d11DeviceContext->VSSetConstantBuffers(0, 1, &commonCBuffer);
	}

	{
		D3D11_BUFFER_DESC animationDesc = {};
		animationDesc.ByteWidth = sizeof(AnimationCBuffer_t) + 0xf & 0xfffffff0;
		animationDesc.Usage = D3D11_USAGE_DYNAMIC;
		animationDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		animationDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = SBEngine::Direct3DResources.d3d11Device->CreateBuffer(&animationDesc, nullptr, &animationCBuffer);
		if (FAILED(hResult)) {
			Logger.HandleError(__LINE__, __FILE__);
			return false;
		}

		SBEngine::Direct3DResources.d3d11DeviceContext->VSSetConstantBuffers(1, 1, &commonCBuffer);
	}

	{
		D3D11_BUFFER_DESC modelDesc = {};
		modelDesc.ByteWidth = sizeof(ModelMatCBuffer_t) + 0xf & 0xfffffff0;
		modelDesc.Usage = D3D11_USAGE_DYNAMIC;
		modelDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		modelDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = SBEngine::Direct3DResources.d3d11Device->CreateBuffer(&modelDesc, nullptr, &modelCBuffer);
		if (FAILED(hResult)) {
			Logger.HandleError(__LINE__, __FILE__);
			return false;
		}

		SBEngine::Direct3DResources.d3d11DeviceContext->VSSetConstantBuffers(2, 1, &commonCBuffer);
	}

	std::filesystem::directory_iterator DirectoryIterator("Data\\Shaders");
	for (const std::filesystem::directory_entry& Directory : DirectoryIterator) {
		
		// Make sure its a file
		if (Directory.is_directory())
			continue;
		
		// Get path and extension
		const std::string& PathString = Directory.path().generic_string();
		const std::string& Extension = Directory.path().extension().string();
		const std::string& FileName = Directory.path().filename().string();
		const std::string& ShaderName = Directory.path().filename().string().substr(3, Directory.path().filename().string().find_last_of('.') - 3);

		// Make sure its a shader
		if (strcmp(Extension.c_str(), ".hlsl") != 0)
			continue;
		
#ifdef _DEBUG
		const static UINT shaderCompileFlags = D3DCOMPILE_DEBUG;
#else
		const static UINT shaderCompileFlags = 0;
#endif // _DEBUG

		if (!shadersMap.contains(FileName))
			shadersMap[FileName] = {};

		Shader_t& loadedShader = shadersMap[FileName];
		loadedShader.shaderName = FileName;

		std::wstring wPath(PathString.begin(), PathString.end());

		// Check if the file starts with 'vs_', its a Vertex-Shader
		if (FileName.starts_with("vs_")) {
			ID3DBlob* vsBlob;
			ID3DBlob* shaderCompileErrorsBlob;

			HRESULT hResult = D3DCompileFromFile(wPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", shaderCompileFlags, 0, &vsBlob, &shaderCompileErrorsBlob);
			if (FAILED(hResult)) {
				Logger.ConsoleLog("Failed to compile vertex-shader", Logger_t::LogType_ERROR);
				const char* errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
				std::cout << errorString << std::endl;;
			} else {
				Logger.ConsoleLog("Compiled vertex-shader", Logger_t::LogType_DEFAULT);
				SBEngine::Direct3DResources.d3d11Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &loadedShader.VSShader);
			}

			// Create the Input Layout for the Shader
			constexpr D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
				{ "POS",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEX",  0, DXGI_FORMAT_R32G32_FLOAT,    0, sizeof(float) * 3,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * (3 + 2),    D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			SBEngine::Direct3DResources.d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &loadedShader.inputLayout);
			vsBlob->Release();
		}

		// Check if the file starts with 'ps_', its a Pixel-Shader
		if (FileName.starts_with("ps_")) {
			ID3DBlob* psBlob;
			ID3DBlob* shaderCompileErrorsBlob;

			HRESULT hResult = D3DCompileFromFile(wPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", shaderCompileFlags, 0, &psBlob, &shaderCompileErrorsBlob);
			if (FAILED(hResult)) {
				Logger.ConsoleLog("Failed to compile pixel-shader", Logger_t::LogType_ERROR);
				const char* errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
				std::cout << errorString << std::endl;;
			} else {
				Logger.ConsoleLog("Compiled pixel-shader", Logger_t::LogType_DEFAULT);
				SBEngine::Direct3DResources.d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &loadedShader.PSShader);
			}
		}
	}
	return true;
}
