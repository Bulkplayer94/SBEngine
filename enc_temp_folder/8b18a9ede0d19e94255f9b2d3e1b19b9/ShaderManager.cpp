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
	std::filesystem::directory_iterator DirectoryIterator("Data\\Shaders");
	for (const std::filesystem::directory_entry& Directory : DirectoryIterator) {
		
		// Make sure its a file
		if (Directory.is_directory())
			continue;
		
		// Get path and extension
		const std::string& PathString = Directory.path().generic_string();
		const std::string& Extension = Directory.path().extension().string();
		const std::string& FileName = Directory.path().filename().string();
		
		// Make sure its a shader
		if (strcmp(Extension.c_str(), ".hlsl") != 0)
			continue;
		
		// Check if its a pixel shader
		bool IsPixelShader = FileName.starts_with("PS_");
		bool IsVertexShader = FileName.starts_with("VS_");

		// Create Vertex Shader
		ID3DBlob* VsBlob;

		UINT ShaderCompileFlags = 0;
		// Compiling with this flag allows debugging shaders with Visual Studio
		#if defined(_DEBUG)
		ShaderCompileFlags |= D3DCOMPILE_DEBUG;
		#endif
			
		SHADER Default;
		Default.ShaderName = FileName;

		std::cout << PathString << std::endl;

		ID3DBlob* ShaderCompileErrorsBlob;

		std::wstring wPath(PathString.begin(), PathString.end());

		HRESULT hResult = D3DCompileFromFile(wPath.c_str(), nullptr, nullptr, "main", "vs_5_0", ShaderCompileFlags, 0, &VsBlob, &ShaderCompileErrorsBlob);
		if (FAILED(hResult)) {
			Logger.ConsoleLog("Failed to compile shader", Logger_t::LogType_ERROR);
		} else {
			Logger.ConsoleLog("Compiled shader", Logger_t::LogType_DEFAULT);
			SBEngine::Direct3DResources.d3d11Device->CreateVertexShader(VsBlob->GetBufferPointer(), VsBlob->GetBufferSize(), nullptr, Default.VSShader);
		}
		
	}

	return true;
}
