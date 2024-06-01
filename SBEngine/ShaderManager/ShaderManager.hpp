#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>
#include <string>
#include <map>

class ShaderManager_t {
private:
	struct Shader_t {
		std::string shaderName;

		ID3D11VertexShader* VSShader;
		ID3D11PixelShader* PSShader;
		ID3D11InputLayout* inputLayout;
	};

	std::map<std::string, Shader_t> shadersMap;

	// Constant Buffer for WorldMatrix, ViewMatrix, PerspectiveMatrix
	struct CommonCBuffer_t {
		DirectX::XMFLOAT4X4 viewMat;
		DirectX::XMFLOAT4X4 projectionMat;
		DirectX::XMFLOAT4X4 viewProjMat;
	};
	ID3D11Buffer* commonCBuffer; // Register b0

	// Animation Buffer for anything that could be usefull while Animating
	struct AnimationCBuffer_t {
		float deltaTime;
	};
	ID3D11Buffer* animationCBuffer; // Register b1
	
	// Model Matrix Buffer for the modelViewProj
	struct ModelMatCBuffer_t {
		DirectX::XMFLOAT4X4 modelMat;
	};
	ID3D11Buffer* modelCBuffer; // Register b2

	/*
		t0 = Volume Map
		t1 = Diffuse Map
		t2 = Normal Map

		s0 = Texture Sampler
	*/
public:

	bool SetContext(const std::string& ShaderName);

	bool StartCompiling();

} extern ShaderManager;