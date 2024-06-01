#include "Camera.hpp"
#include "../MathDefinitions.hpp"
#include "../ImGui/imgui.h"

using namespace SBEngine;

Camera_t SBEngine::Camera = {};

bool Camera_t::Proc(float deltaTime) {

	using namespace DirectX;

	XMVECTOR _pos = XMLoadFloat3(&this->pos);
	XMVECTOR _ang = XMLoadFloat3(&this->ang);
	{
		float moveAmount = 100.0F * deltaTime;
		XMVECTOR fwdXZ = XMVector3Normalize({ fwd.x, 0, fwd.z });
		XMVECTOR fwdRightXZ = XMVector3Cross(fwdXZ, { 0.0F, 1.0F, 0.0F });

		if (ImGui::IsKeyDown(ImGuiKey_W))
			_pos += fwdXZ * moveAmount;
		if (ImGui::IsKeyDown(ImGuiKey_S))
			_pos -= fwdXZ * moveAmount;
		if (ImGui::IsKeyDown(ImGuiKey_D))
			_pos += fwdRightXZ * moveAmount;
		if (ImGui::IsKeyDown(ImGuiKey_A))
			_pos -= fwdRightXZ * moveAmount;

		if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
			_ang += {0.0F, 0.0F, moveAmount};
		if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
			_ang -= {0.0F, 0.0F, moveAmount};
		if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
			_ang += {0.0F, moveAmount, 0.0F};
		if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
			_ang -= {0.0F, moveAmount, 0.0F};

		if (ImGui::IsKeyDown(ImGuiKey_Q))
			_pos += {0.0F, moveAmount, 0.0F};
		if (ImGui::IsKeyDown(ImGuiKey_E))
			_pos -= {0.0F, moveAmount, 0.0F};
		
		XMStoreFloat3(&this->pos, _pos);
		XMStoreFloat3(&this->ang, _ang);
	}

	XMMATRIX _viewMat = DirectX::XMMatrixLookAtLH(_pos, _ang, upDir);
	XMStoreFloat4x4(&this->viewMat, _viewMat);

	fwd = { -viewMat.m[2][0], -viewMat.m[2][1], -viewMat.m[2][2] };

	return true;
}