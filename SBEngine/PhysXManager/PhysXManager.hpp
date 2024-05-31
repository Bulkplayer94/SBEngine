#pragma once

#include <PxPhysicsAPI.h>

struct PhysXManager_t {

	PxFoundation* mFoundation;
	PxPvd* mPvd;
	PxPhysics* mPhysics;
	PxPvdTransport* mTransport;
	PxScene* mScene;
	PxMaterial* mMaterial;

	bool SetupPhysX();

} extern PhysXManager;