#pragma once
#include <DirectXMath.h>

namespace SBEngine {
	struct Camera_t {
		DirectX::XMFLOAT4X4 viewMat;

		DirectX::XMFLOAT3 pos = {1.0F, 1.0F, 1.0F};
		DirectX::XMFLOAT3 fwd = {-0.02F, -0.076F, -1.0F};
		DirectX::XMFLOAT3 ang = {0.0F, 0.0F, 0.0F};

		bool Proc(float deltaTime);
	} extern Camera;
}