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
        if (!g_RaycastFunc) return false;
        return g_RaycastFunc(start, dir, range, outHitPoint, outNormal);
    }

    void LedgeDetector::Update()
    {
        const auto player = RE::PlayerCharacter::GetSingleton();
        const auto camera = RE::PlayerCamera::GetSingleton();
        if (!player || !camera) return;

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
        // Implementation for HUDMarkerData projection
        // In F4, updating HUD markers usually requires accessing the HUDManager
        // and modifying an existing marker's location.
    }
}
