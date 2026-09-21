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
#include <cmath>

namespace Traversal
{
    LedgeDetector* LedgeDetector::GetSingleton()
    {
        static LedgeDetector singleton;
        return &singleton;
    }

    bool LedgeDetector::PerformRaycast(const RE::NiPoint3& start, const RE::NiPoint3& dir, float range, RE::NiPoint3& outHitPoint, RE::NiPoint3& outNormal)
    {
        using GetBhkWorldFunc = RE::bhkWorld*(*)();
        static REL::Relocation<GetBhkWorldFunc> g_GetBhkWorld{ RE::ID::bhkWorld::GetbhkWorld };

        auto bhkWorld = g_GetBhkWorld();
        if (!bhkWorld) return false;

        auto npWorld = bhkWorld->m_worldNP.get();
        if (!npWorld) return false;

        RE::hknpRayCastQuery query;
        query.m_ray.m_origin = { start.x, start.y, start.z, 1.0f };
        query.m_ray.m_direction = { dir.x, dir.y, dir.z, 0.0f };

        RE::hknpRayCastQueryResult result;
        if (npWorld->castRay(query, result))
        {
            outHitPoint = { result.m_hitPoint.x, result.m_hitPoint.y, result.m_hitPoint.z };
            outNormal = { result.m_hitNormal.x, result.m_hitNormal.y, result.m_hitNormal.z };

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

        if (m_climbState == ClimbState::Interpolating)
        {
            float deltaTime = 0.016f; 
            m_interpolationTimer += deltaTime;

            float t = m_interpolationTimer / kClimbDuration;
            if (t >= 1.0f)
            {
                player->SetPosition(m_targetPos, true);
                m_climbState = ClimbState::Idle;
                REX::INFO("Climb Completed");
            }
            else
            {
                RE::NiPoint3 currentPos = player->GetPosition();
                RE::NiPoint3 lerpedPos;

                if (t < 0.5f) {
                    float zT = t * 2.0f;
                    lerpedPos.x = currentPos.x;
                    lerpedPos.y = currentPos.y;
                    lerpedPos.z = currentPos.z + (m_targetPos.z - currentPos.z) * zT;
                } else {
                    float xyT = (t - 0.5f) * 2.0f;
                    lerpedPos.x = currentPos.x + (m_targetPos.x - currentPos.x) * xyT;
                    lerpedPos.y = currentPos.y + (m_targetPos.y - currentPos.y) * xyT;
                    lerpedPos.z = m_targetPos.z;
                }
                player->SetPosition(lerpedPos, true);
            }
            return;
        }

        float yaw = player->GetAngle(); 
        RE::NiPoint3 forwardVec;
        forwardVec.x = std::sin(yaw);
        forwardVec.y = std::cos(yaw);
        forwardVec.z = 0.0f;

        RE::NiPoint3 startPos = player->GetPosition();
        startPos.z += 40.0f;

        RE::NiPoint3 wallHit, wallNormal;
        if (PerformRaycast(startPos, forwardVec, m_maxReach, wallHit, wallNormal))
        {
            RE::NiPoint3 probeStart = wallHit;
            probeStart.x -= wallNormal.x * 5.0f;
            probeStart.y -= wallNormal.y * 5.0f;
            probeStart.z += 100.0f;

            RE::NiPoint3 downVec = { 0, 0, -1 }; 
            RE::NiPoint3 ledgeHit, ledgeNormal;

            if (PerformRaycast(probeStart, downVec, 150.0f, ledgeHit, ledgeNormal))
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
