#include "PhysXManager.hpp"
#include <iostream>
#include <Windows.h>

using namespace SBEngine;

SBEngine::PhysXManager_t SBEngine::PhysXManager;

static bool ConsoleEventOccured = false;

FILE* stream2;

using namespace physx;
class PhysXErrorCallback : public PxErrorCallback {
	void reportError(PxErrorCode::Enum code, const char* msg, const char* file, int line) {

		if (!ConsoleEventOccured) {
#ifdef _DEBUG
			AllocConsole();
            freopen_s(&stream2, "conout$", "w", stdout);
#else
            freopen_s(&stream2, "error.log", "w", stdout);
#endif // _DEBUG
		}

		std::cout << "An PhysX Error Occured!\n" << "In File: " << file << "\n" << "on Line: " << line << "\n" << msg << std::endl;
	}
};

bool PhysXManager_t::SetupPhysX() {
    static PhysXErrorCallback gErrorCallback;
    static PxDefaultAllocator gDefaultAllocatorCallback;

    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gErrorCallback);
    if (!mFoundation) {
        std::cout << "PhysX Loading Failed!" << std::endl;
        return false;
    }

    bool recordMemoryAllocation = true;

    mPvd = PxCreatePvd(*mFoundation);
    mTransport = PxDefaultPvdFileTransportCreate("E:\\GitProjects\\BEngine\\PxSaved.pvd");
    mPvd->connect(*mTransport, PxPvdInstrumentationFlag::eALL);

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), recordMemoryAllocation, mPvd);
    if (!mPhysics) {
        std::cout << "PxPhysics didnt Initialize!" << std::endl;
        return false;
    }
        

    if (!PxInitExtensions(*mPhysics, mPvd)) {
        std::cout << "PxExtension Failed!" << std::endl;
        return false;
    }

    mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);

    if (!mMaterial) {
        std::cout << "Material creation Failed!" << std::endl;
        return false;
    }

    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(6);
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    mScene = mPhysics->createScene(sceneDesc);
    if (!mScene) {
        std::cout << "PxScene creation failed!" << std::endl;
        return false;
    }

    PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, PxPlane(0.0f, 1.0f, 0.0f, 0.0f), *mMaterial);
    PxTransform transform = groundPlane->getGlobalPose();
    transform.p.y = -100.0f;
    groundPlane->setGlobalPose(transform);
    mScene->addActor(*groundPlane);

    return true;
}
