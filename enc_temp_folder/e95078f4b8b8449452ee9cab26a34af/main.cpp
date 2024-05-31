#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")


#include "globals.h"
#include "GuiLib.h"

#include <iostream>
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <stdint.h>

#include "shader/SHADER_default.h"
#include "IMOverlayManager.h"
#include "EntityManager.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_stdlib.h"

#include "PhysXManager.h"

#include "MeshManager.h"
#include "LuaManager.h"

#include "ErrorReporter.h"

#include <chrono>

#include <thread>

FILE* stream;

bool isLoading = true;

using namespace physx;

class ExplosionCallback : public PxSweepCallback {
public:
    ExplosionCallback(PxSweepHit* hits, PxU32 maxNbHits)
        : PxSweepCallback(hits, maxNbHits) {}

    virtual PxAgain processTouches(const PxSweepHit* buffer, PxU32 nbHits) override {
        for (PxU32 i = 0; i < nbHits; ++i) {
            const PxSweepHit& hit = buffer[i];
           
            PxTransform actorPos = hit.actor->getGlobalPose();

            std::cout << std::format("Hit Actor at {}, {}, {}", actorPos.p.x, actorPos.p.y, actorPos.p.z) << std::endl;

            PxRigidDynamic* act = (PxRigidDynamic*)hit.actor;

            PxVec3 force = actorPos.p - explosionCenter;
            force *= (500 / (force.magnitude() + 1));

            act->addForce(force, PxForceMode::eIMPULSE);

        }
        return true;
    }

    PxVec3 explosionCenter;
    
};

void performExplosion(PxScene* scene, const PxVec3& explosionCenter, float explosionStrength, float explosionRadius) {
    PxSphereGeometry sphereGeom(explosionRadius);

    const PxU32 maxNbHits = 256;
    PxSweepHit hitBuffer[maxNbHits];

    ExplosionCallback sweepCallback(hitBuffer, maxNbHits);
    sweepCallback.explosionCenter = explosionCenter;

    PxQueryFilterData filterData;
    filterData.flags = PxQueryFlag::eDYNAMIC;

    if (scene->sweep(sphereGeom, PxTransform(explosionCenter), PxVec3(1.0f, 0.0f, 0.0f), 0, sweepCallback, PxHitFlag::eDEFAULT, filterData)) {
        std::cout << "Sweep found " << sweepCallback.nbTouches << " hits." << std::endl;

        //for (PxU32 i = 0; i < sweepCallback.nbTouches; ++i) {
        //    PxRigidDynamic* dynamicActor = sweepCallback.touches[i].actor->is<PxRigidDynamic>();
        //    if (dynamicActor) {
        //        PxVec3 actorPos = dynamicActor->getGlobalPose().p;
        //        PxVec3 forceDirection = actorPos - explosionCenter;
        //        float distance = forceDirection.normalize();

        //        if (distance < explosionRadius) {
        //            float forceMagnitude = explosionStrength * (1.0f - (distance / explosionRadius));
        //            PxVec3 force = forceDirection * forceMagnitude;
        //            dynamicActor->addForce(force, PxForceMode::eIMPULSE);
        //        }
        //    }
        //}
    }
    else {
        std::cout << "No hits found." << std::endl;
    }
}


void LoadRessources() {
    
    BEngine::meshManager.StartLoading();

    const unsigned int lenght = 10;
    const float startingPosition = -(static_cast<float>(lenght) / 2);

    for (unsigned int i = 0; i != lenght; ++i) {
        for (unsigned int i2 = 0; i2 != lenght; ++i2) {
            for (unsigned int i3 = 0; i3 != lenght; ++i3) {
                Entity* positionedEnt = entityManager.RegisterEntity(BEngine::meshManager.meshList["cube"], false);
                positionedEnt->SetPosition({ startingPosition + (i * 5.0F), 0.0F + (i3 * 5.0F), startingPosition + (i2 * 5.0F)});
            }
            
        }
    }

    //Entity* spawnedWareHouse = entityManager.RegisterEntity(BEngine::meshManager.meshList["warehouse"], true);

    //entityManager.RegisterEntity(BEngine::meshManager.meshList["base_plattform"], true, { 0.0F, -10.0F, 0.0F });
    //entityManager.RegisterEntity(BEngine::meshManager.meshList["ball"], false);

    //entityManager.RegisterEntity(BEngine::meshManager.meshList["galil"], false, { 0.0F, 0.0F, 0.0F });

    isLoading = false;

    return;

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    freopen_s(&stream, "debug.log", "w", stdout);
    Globals::initGlobals(hInstance);

    PhysXManager::SetupPhysX();

    using namespace Globals::Direct3D;
    using namespace Globals::Win32;
    using namespace Globals::CUserCmd;
    using namespace physx;

    std::thread thr(&LoadRessources);
    thr.detach();

    IMOverlayManager imOverlayManager;

    //BEngine::LoadAdvancedShaders(d3d11Device);

    const SHADER& DefaultShades = SHADER_DEFAULT::Load(Globals::Direct3D::d3d11Device);

    // Camera
    float3 cameraPos = { 0.0F, 0.0F, 0.0F };
    float3 cameraFwd = { -0.02F, -0.076F, -1.0F };
    float cameraPitch = 0.f;
    float cameraYaw = 0.f;
    unsigned int cameraWpn = 0;

    float4x4 perspectiveMat = {};
    Globals::Status::windowStatus[Globals::Status::WindowStatus_RESIZE] = true; // To force initial perspectiveMat calculation

    // Timing
    LONGLONG startPerfCount = 0;
    LONGLONG perfCounterFrequency = 0;
    {
        LARGE_INTEGER perfCount;
        QueryPerformanceCounter(&perfCount);
        startPerfCount = perfCount.QuadPart;
        LARGE_INTEGER perfFreq;
        QueryPerformanceFrequency(&perfFreq);
        perfCounterFrequency = perfFreq.QuadPart;
    }
    long double currentTimeInSeconds = 0.0L;

    std::cout << "Devices" << std::endl << \
        d3d11Device << std::endl << \
        d3d11DeviceContext << std::endl << \
        d3d11SwapChain << std::endl; 

    BEngine::luaManager.Init();

    ImVec2 MousePos = { 0.0F, 0.0F };

    // Main Loop
    bool isRunning = true;
    while (isRunning)
    {

        float dt;
        {
            double previousTimeInSeconds = currentTimeInSeconds;
            LARGE_INTEGER perfCount;
            QueryPerformanceCounter(&perfCount);

            currentTimeInSeconds = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;
            dt = (float)(currentTimeInSeconds - previousTimeInSeconds);
            //if (dt > (1.f / 60.f))
            //    dt = (1.f / 60.f);
        }

        MSG msg = {};
        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                isRunning = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        auto start = std::chrono::high_resolution_clock::now();
        if (!isLoading) {
            Globals::PhysX::mScene->simulate(dt);
            Globals::PhysX::mScene->fetchResults(true);
        }
        long long mögliche_leistung = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

        float3 eyeTracePos = { 0.0F, 0.0F, 0.0F };
        if (!isLoading) {
            PxVec3 origin = { cameraPos.x, cameraPos.y, cameraPos.z };
            PxVec3 unitDir = { cameraFwd.x, cameraFwd.y, cameraFwd.z };
            PxReal maxDistance = 500.0F;
            PxRaycastBuffer hit;

            unitDir.normalize();

            bool status = Globals::PhysX::mScene->raycast(origin, unitDir, maxDistance, hit);
            if (status) {
                eyeTracePos = { hit.block.position.x, hit.block.position.y, hit.block.position.z };

                if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft, false) && (hit.block.actor->getType() == physx::PxActorType::eRIGID_DYNAMIC)) {
                    PxRigidDynamic* act = (PxRigidDynamic*)hit.block.actor;

                    PxVec3 forceDir = hit.block.position - act->getGlobalPose().p;
                    forceDir.normalize();
                    forceDir *= -1;

                    PxReal forceMagnitude = 2500.0f;
                    act->addForce(forceDir * forceMagnitude, physx::PxForceMode::eIMPULSE);
                }

                if (ImGui::IsKeyPressed(ImGuiKey_MouseRight, false)) {
                    PxVec3 explosionCenter = { eyeTracePos.x, eyeTracePos.y, eyeTracePos.z };
                    float explosionStrength = 2500.0f;
                    float explosionRadius = 50.0f;

                    performExplosion(Globals::PhysX::mScene, explosionCenter, explosionStrength, explosionRadius);
                }
            }

            if (ImGui::IsKeyPressed(ImGuiKey_P)) {
                Entity* entity = entityManager.RegisterEntity(BEngine::meshManager.meshList["cube"], false);
                PxTransform trans = entity->physicsActor->getGlobalPose();

                trans.p.x = cameraPos.x;
                trans.p.y = cameraPos.y;
                trans.p.z = cameraPos.z;

                entity->physicsActor->setGlobalPose(trans);

                PxRigidDynamic* dyn = (PxRigidDynamic*)entity->physicsActor;
                dyn->addForce(unitDir * 1000.0F, physx::PxForceMode::eIMPULSE);
            }

            if (ImGui::IsKeyPressed(ImGuiKey_1)) {
                Entity* spawned_ent = entityManager.RegisterEntity(BEngine::meshManager.meshList["cube"], false, { cameraPos.x, cameraPos.y, cameraPos.z });
                

            }

            Globals::PhysX::mPlayerController->move(PxVec3(0.0F, 0.0F, 2.0F), 1.0F, dt, PxControllerFilters());
        }

        imOverlayManager.Proc();

        static bool mouseWasReleased = true;
        ImVec2 realMouseDrag = { 0,0 };
        {
            POINT mousePoint;
            if (GetCursorPos(&mousePoint)) {

                RECT hwndInfo;
                RECT clientRect;
                if (GetWindowRect(hWnd, &hwndInfo) && GetClientRect(hWnd, &clientRect)) {
                    int HWNDwindowWidth = hwndInfo.left;
                    int HWNDwindowHeight = hwndInfo.top;
                    int _windowWidth = clientRect.right - clientRect.left;
                    int _windowHeight = clientRect.bottom - clientRect.top;

                    ImVec2 newMousePos = { 0.0F,0.0F };

                    //ScreenToClient(hWnd, &mousePoint);

                    newMousePos.x = (float)mousePoint.x;
                    newMousePos.y = (float)mousePoint.y;

                    realMouseDrag = { (HWNDwindowWidth + (_windowWidth / 2))  - newMousePos.x, (HWNDwindowHeight + (_windowHeight / 2)) - newMousePos.y};

                    MousePos = newMousePos;


                    //bool lastWindowStatus = true;
                    if (!Globals::Status::windowStatus[Globals::Status::WindowStatus_PAUSED]) {
                        //SetCursor(NULL);
                    }
                    else { 
                        //HCURSOR SetCursor(LoadCursorW(0, IDC_ARROW));
                    }
                    
                }
            }
        } 

        // Get window dimensions
        float windowWidth, windowHeight;
        float windowAspectRatio;
        {
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            windowWidth = clientRect.right - clientRect.left;
            windowHeight = clientRect.bottom - clientRect.top;
            windowAspectRatio = (float)windowWidth / (float)windowHeight;

            if (!Globals::Status::windowStatus[Globals::Status::WindowStatus_PAUSED] && GetActiveWindow() == hWnd && GetFocus() == hWnd) {
                RECT hwndInfo;
                GetWindowRect(hWnd, &hwndInfo);
                int HWNDwindowWidth = hwndInfo.left;
                int HWNDwindowHeight = hwndInfo.top;
                
                SetCursorPos(HWNDwindowWidth + (windowWidth / 2), HWNDwindowHeight + (windowHeight / 2));
                //std::cout << HWNDwindowWidth << " : " << HWNDwindowHeight << std::endl;
            }
        }

        if (Globals::Status::windowStatus[Globals::Status::WindowStatus_RESIZE] == true)
        {
            d3d11DeviceContext->OMSetRenderTargets(0, 0, 0);
            d3d11FrameBufferView->Release();
            depthBufferView->Release();

            HRESULT res = d3d11SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            assert(SUCCEEDED(res));

            win32CreateD3D11RenderTargets(d3d11Device, d3d11SwapChain, &d3d11FrameBufferView, &depthBufferView);
            perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(84), 0.1f, 1000.f);

            Globals::Status::windowStatus[Globals::Status::WindowStatus_RESIZE] = false;
        } 

        static float mouseSensitivity = 20.0F;

        // Update camera
        if (!Globals::Status::windowStatus[Globals::Status::WindowStatus_PAUSED])
        {
            float3 camFwdXZ = normalise(float3{ cameraFwd.x, 0, cameraFwd.z });
            float3 cameraRightXZ = cross(camFwdXZ, { 0, 1, 0 });

            float CAM_MOVE_SPEED = 5.f; // in metres per second
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
                CAM_MOVE_SPEED *= 4;

            const float CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * dt;
            if (ImGui::IsKeyDown(ImGuiKey_W))
                cameraPos += camFwdXZ * CAM_MOVE_AMOUNT;
            if (ImGui::IsKeyDown(ImGuiKey_S))
                cameraPos -= camFwdXZ * CAM_MOVE_AMOUNT;
            if (ImGui::IsKeyDown(ImGuiKey_A))
                cameraPos -= cameraRightXZ * CAM_MOVE_AMOUNT;
            if (ImGui::IsKeyDown(ImGuiKey_D))
                cameraPos += cameraRightXZ * CAM_MOVE_AMOUNT;
            if (ImGui::IsKeyDown(ImGuiKey_E))
                cameraPos.y += CAM_MOVE_AMOUNT;
            if (ImGui::IsKeyDown(ImGuiKey_Q))
                cameraPos.y -= CAM_MOVE_AMOUNT;

            if (mouseWasReleased == false) {
                mouseWasReleased = true;
            } else {
                cameraYaw += realMouseDrag.x * (mouseSensitivity / 10000);
                cameraPitch += realMouseDrag.y * (mouseSensitivity / 10000);
            }

            // Wrap yaw to avoid floating-point errors if we turn too far
            while (cameraYaw >= 2.0F * (float)M_PI)
                cameraYaw -= 2.0F * (float)M_PI;
            while (cameraYaw <= -2.0F * (float)M_PI)
                cameraYaw += 2.0F * (float)M_PI;

            // Clamp pitch to stop camera flipping upside down
            if (cameraPitch > degreesToRadians(85))
                cameraPitch = degreesToRadians(85);
            if (cameraPitch < -degreesToRadians(85))
                cameraPitch = -degreesToRadians(85);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            Globals::Status::windowStatus[Globals::Status::WindowStatus_PAUSED] = !Globals::Status::windowStatus[Globals::Status::WindowStatus_PAUSED];
            mouseWasReleased = true;
        }

        float4x4 viewMat = translationMat(-cameraPos) * rotateYMat(-cameraYaw) * rotateXMat(-cameraPitch);
        cameraFwd = { -viewMat.m[2][0], -viewMat.m[2][1], -viewMat.m[2][2] };

        FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);

        d3d11DeviceContext->ClearDepthStencilView(depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)windowWidth, (FLOAT)windowHeight, 0.0f, 1.0f };
        d3d11DeviceContext->RSSetViewports(1, &viewport);

        d3d11DeviceContext->RSSetState(rasterizerState);
        d3d11DeviceContext->OMSetDepthStencilState(depthStencilState, 0);

        d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, depthBufferView);

        d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3d11DeviceContext->PSSetSamplers(1, 1, &samplerState);

        if (!isLoading) {
            entityManager.Draw(const_cast<SHADER*>(&DefaultShades), & BEngine::meshManager, & viewMat, & perspectiveMat);
        }

        if (isLoading)
        {
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            ImGuiIO& io = ImGui::GetIO();

            ImVec2 textSize = ImGui::CalcTextSize("Loading...");

            drawList->AddRectFilled(ImVec2(0, 0), io.DisplaySize, ImColor(0, 0, 0));
            drawList->AddText(ImVec2((io.DisplaySize.x / 2) - (textSize.x / 2), (io.DisplaySize.y / 2) - (textSize.y / 2)), ImColor(255, 255, 255), "Loading...");
            
        }
        else {
            ImGui::Begin("Camera");
            {
                std::string delta = "DeltaTime: " + std::to_string(dt);
                ImGui::Text(delta.c_str());

                static float smoothFPS = 60.0F;
                smoothFPS -= (smoothFPS - (1.0F / dt)) * dt * 2;

                static float counter = 0;
                counter += dt;

                static float maxFPS = 0.0F;
                static float minFPS = 1000000000.0F;
                const float FPS = (1.0F / dt);

                if (counter > 0.5F) {
                    if (maxFPS < smoothFPS)
                        maxFPS = smoothFPS;

                    if (minFPS > smoothFPS)
                        minFPS = smoothFPS;
                }
                

                ImGui::Text("Max FPS: %.4f", maxFPS);
                ImGui::Text("Min FPS: %.4f", minFPS);
                


                ImGui::Text("FPS: %.2f", smoothFPS);
                ImGui::Text("Pos: %.2f, %.2f, %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
                ImGui::Text("Fwd: %.2f, %.2f, %.2f", cameraFwd.x, cameraFwd.y, cameraFwd.z);
                ImGui::Text("Pit: %.2f", cameraPitch);
                ImGui::Text("Yaw: %.2f", cameraYaw);

                ImGui::SliderFloat("Maus Empfindlichkeit", &mouseSensitivity, 0, 100);

                ImGui::NewLine();

                ImGui::Text("Actor Num Static: %d", (unsigned int)Globals::PhysX::mScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC));
                ImGui::Text("Actor Num Dynamic: %d", (unsigned int)Globals::PhysX::mScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC));

                ImGui::Text("M\xc3\xb6gliche Leistung: %f", 1.0F / (static_cast<float>(mögliche_leistung) / 10000000.0F));
                ImGui::Text("Eyetrace Position: %.2f, %.2f, %.2f", eyeTracePos.x, eyeTracePos.y, eyeTracePos.z );

                ImDrawList* bgList = ImGui::GetBackgroundDrawList();
                float2 projectionPoint = BEngine::GuiLib::project3Dto2D(eyeTracePos, viewMat, perspectiveMat, windowWidth, windowHeight);
                bgList->AddCircle({projectionPoint.x, projectionPoint.y}, 2.0F, ImColor(255, 0, 0, 100));

                PxExtendedVec3 playerPos = Globals::PhysX::mPlayerController->getPosition();
                float2 playerProjection = BEngine::GuiLib::project3Dto2D({ (float)playerPos.x, (float)playerPos.y, (float)playerPos.z }, viewMat, perspectiveMat, windowWidth, windowHeight);
                bgList->AddCircle({ playerProjection.x, playerProjection.y }, 2.0F, ImColor(0, 255, 0, 100));

            }
            ImGui::End();
        }

        BEngine::errorReporter.Draw();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        d3d11SwapChain->Present(0, 0);
    }

    return 0;
}
