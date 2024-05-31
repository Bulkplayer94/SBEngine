#pragma once

#include <PxPhysicsAPI.h>

namespace SBEngine {
	struct PhysXManager_t {
		physx::PxFoundation* mFoundation = nullptr;
		physx::PxPvd* mPvd = nullptr;
		physx::PxPhysics* mPhysics = nullptr;
		physx::PxPvdTransport* mTransport = nullptr;
		physx::PxScene* mScene = nullptr;
		physx::PxMaterial* mMaterial = nullptr;
	
		bool SetupPhysX();
	
	} extern PhysXManager;
}
