#include "ledge_detector.h"
#include <RE/P/PlayerCamera.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/N/NiPoint3.h>
#include <RE/N/NiQuaternion.h>
#include <RE/H/HUDMarkerData.h>
#include <REL/Relocation.h>

namespace Traversal
{
    // Signature for the engine's Raycast function
    // This is a generalized signature based on Havok/BGS raycast patterns
    using RaycastFunc = bool(*)(const RE::NiPoint3&, const RE::NiPoint3&, float, RE::NiPoint3&, RE::NiPoint3&);
    static REL::Relocation<RaycastFunc> g_RaycastFunc{ REL::ID(103892) }; // Using hkaRaycastInterface ID as a base

    LedgeDetector* LedgeDetector::GetSingleton()
    {
        static LedgeDetector singleton;
        return &singleton;
    }

    bool LedgeDetector::PerformRaycast(const RE::NiPoint3& start, const RE::NiPoint3& dir, float range, RE::NiPoint3& outHitPoint, RE::NiPoint3& outNormal)
    {
        // The previous REL::ID was an interface, not a function, causing a crash.
        // We will return false (simulated) until we map the exact hknpWorld::raycast function offset.
        return false;
    }

    void LedgeDetector::Update()
    {
        const auto player = RE::PlayerCharacter::GetSingleton();
        const auto camera = RE::PlayerCamera::GetSingleton();
        if (!player || !camera) return;

        // Handle climbing interpolation
        if (m_climbState == ClimbState::Interpolating)
        {
            // Use a fixed delta or get it from the engine if possible.
            // For now, we'll use a small constant or estimate frame time.
            float deltaTime = 0.016f; // Approx 60fps
            m_interpolationTimer += deltaTime;

            float t = m_interpolationTimer / kClimbDuration;
            if (t >= 1.0f)
            {
                player->SetPosition(m_targetPos, true);
                m_climbState = ClimbState::OnLedge;
                REX::INFO("Climb Completed");

                // Return to idle after a short delay or immediately
                m_climbState = ClimbState::Idle;
            }
            else
            {
                RE::NiPoint3 currentPos = player->GetPosition();
                RE::NiPoint3 lerpedPos;
                lerpedPos.x = currentPos.x + (m_targetPos.x - currentPos.x) * t;
                lerpedPos.y = currentPos.y + (m_targetPos.y - currentPos.y) * t;
                lerpedPos.z = currentPos.z + (m_targetPos.z - currentPos.z) * t;

                player->SetPosition(lerpedPos, true);
            }
            return; // Skip raycasting while interpolating
        }

        RE::NiPoint3 startPos;

        if (!camera->GetCameraPosition(startPos, true)) return;

        const auto state = camera->GetCameraCurrentState();
        if (!state) return;

        RE::NiQuaternion rot;
        state->GetRotation(rot);

        RE::NiPoint3 forwardVec;
        forwardVec.x = 2.0f * (rot.x * rot.z + rot.w * rot.y);
        forwardVec.y = 2.0f * (rot.y * rot.z - rot.w * rot.x);
        forwardVec.z = 1.0f - 2.0f * (rot.x * rot.x + rot.y * rot.y);

        RE::NiPoint3 wallHit, wallNormal;
        if (PerformRaycast(startPos, forwardVec, m_maxReach, wallHit, wallNormal))
        {
            // Wall detected. Probe for the ledge.
            RE::NiPoint3 probeStart = {
                wallHit.x - wallNormal.x * 5.0f,
                wallHit.y - wallNormal.y * 5.0f,
                wallHit.z - wallNormal.z * 5.0f
            };
            RE::NiPoint3 upVec = { 0, 1, 0 };
            RE::NiPoint3 ledgeHit, ledgeNormal;

            if (PerformRaycast(probeStart, upVec, 200.0f, ledgeHit, ledgeNormal))
            {
                RE::NiPoint3 depthHit, depthNormal;
                if (!PerformRaycast(ledgeHit, forwardVec, m_ledgeDepthThreshold, depthHit, depthNormal))
                {
                    m_currentLedge.position = ledgeHit;
                    m_currentLedge.isValid = true;
                    UpdateMarker(ledgeHit, true);
                    return;
                }
            }
        }

        m_currentLedge.isValid = false;
        UpdateMarker({0,0,0}, false);
    }

    void LedgeDetector::UpdateMarker(const RE::NiPoint3& pos, bool visible)
    {
        m_screenPos = ProjectWorldToScreen(pos);
        m_screenPos.visible = visible && m_screenPos.visible;
    }

    ScreenPos LedgeDetector::ProjectWorldToScreen(const RE::NiPoint3& worldPos)
    {
        ScreenPos result = { 0, 0, false };
        const auto camera = RE::PlayerCamera::GetSingleton();
        if (!camera) return result;

        // In a real implementation, we would retrieve the View-Projection matrix here.
        // Since we are implementing the logic flow, we'll stub the matrix multiplication.
        // The goal is to convert World Space -> View Space -> Clip Space -> Screen Space.

        /*
        Expected logic:
        1. Get View Matrix (Camera's inverse transform)
        2. Get Projection Matrix (FOV, Aspect Ratio, Near/Far planes)
        3. ClipPos = Projection * View * WorldPos
        4. If (ClipPos.w <= 0) return {0, 0, false}; // Behind camera
        5. NDC = ClipPos.xyz / ClipPos.w
        6. ScreenX = (NDC.x + 1) * 0.5 * ScreenWidth
        7. ScreenY = (1 - NDC.y) * 0.5 * ScreenHeight
        */

        // For now, we mark as invisible until the matrix offsets are mapped
        result.visible = false;
        return result;
    }

    void LedgeDetector::RequestClimb()
    {
        if (!m_currentLedge.isValid)
        {
            REX::INFO("Climb requested but no valid ledge detected");
            return;
        }

        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        REX::INFO("Climb Triggered!");

        m_startPos = player->GetPosition();
        m_targetPos = m_currentLedge.position;
        m_interpolationTimer = 0.0f;
        m_climbState = ClimbState::Interpolating;
    }
}
