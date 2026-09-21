#include "ledge_detector.h"
#include <RE/P/PlayerCamera.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/N/NiPoint3.h>
#include <RE/N/NiQuaternion.h>
#include <RE/H/HUDMarkerData.h>
#include <REL/Relocation.h>
#include <RE/B/bhkWorld.h>
#include <RE/H/hknpWorld.h>
#include <RE/H/hknpRayCastQuery.h>
#include <RE/H/hknpRayCastQueryResult.h>

namespace Traversal
{
    LedgeDetector* LedgeDetector::GetSingleton()
    {
        static LedgeDetector singleton;
        return &singleton;
    }

    bool LedgeDetector::PerformRaycast(const RE::NiPoint3& start, const RE::NiPoint3& dir, float range, RE::NiPoint3& outHitPoint, RE::NiPoint3& outNormal)
    {
        // Get the bhkWorld instance
        // Using the REL ID for GetbhkWorld from IDs.h (2200260)
        using GetBhkWorldFunc = RE::bhkWorld*(*)();
        static REL::Relocation<GetBhkWorldFunc> g_GetBhkWorld{ RE::ID::bhkWorld::GetbhkWorld };

        auto bhkWorld = g_GetBhkWorld();
        if (!bhkWorld) return false;

        // Access the narrow-phase world (m_worldNP at 0x60)
        // In CommonLibF4, m_worldNP is a hkRefPtr<hknpBSWorld>
        auto npWorld = bhkWorld->m_worldNP.get();
        if (!npWorld) return false;

        // Prepare the query
        RE::hknpRayCastQuery query;
        query.m_ray.m_origin = { start.x, start.y, start.z, 1.0f };
        query.m_ray.m_direction = { dir.x, dir.y, dir.z, 0.0f };
        // Range is handled by the query's internal distance or by clipping the result

        RE::hknpRayCastQueryResult result;

        // We need the castRay method. Since it's a member of hknpWorld, we can call it if defined.
        // Note: hknpWorld in CommonLibF4 might not have all virtuals wrapped.
        // We'll try to use the member function if it exists, otherwise we'd need a REL offset.
        // Based on agent report, npWorld->castRay(query, result) is the intended call.
        if (npWorld->castRay(query, result))
        {
            // Extract hit point and normal
            // hknpRayCastQueryResult usually contains a fraction and the hit point
            outHitPoint = { result.m_hitPoint.x, result.m_hitPoint.y, result.m_hitPoint.z };
            outNormal = { result.m_hitNormal.x, result.m_hitNormal.y, result.m_hitNormal.z };

            // Check if hit is within range
            float distSq = (outHitPoint.x - start.x) * (outHitPoint.x - start.x) +
                           (outHitPoint.y - start.y) * (outHitPoint.y - start.y) +
                           (outHitPoint.z - start.z) * (outHitPoint.z - start.z);

            if (distSq > range * range) return false;

            return true;
        }

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
