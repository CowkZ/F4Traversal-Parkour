#pragma once

#include <RE/P/PlayerCharacter.h>
#include <RE/N/NiPoint3.h>
#include <RE/N/NiQuaternion.h>
#include <RE/H/HUDMarkerData.h>
#include <REL/Relocation.h>
#include <optional>

namespace Traversal
{
    enum class ClimbState
    {
        Idle,
        Interpolating,
        OnLedge
    };

    struct LedgeInfo
    {
        RE::NiPoint3 position;
        bool isValid = false;
    };

    struct ScreenPos
    {
        float x;
        float y;
        bool visible = false;
    };

    class LedgeDetector
    {
    public:
        static LedgeDetector* GetSingleton();

        void Update();
        void RequestClimb();

        const LedgeInfo& GetCurrentLedge() const { return m_currentLedge; }
        ScreenPos GetLedgeScreenPos() const { return m_screenPos; }

    private:
        LedgeDetector() = default;

        bool PerformRaycast(const RE::NiPoint3& start, const RE::NiPoint3& dir, float range, RE::NiPoint3& outHitPoint, RE::NiPoint3& outNormal);
        void UpdateMarker(const RE::NiPoint3& pos, bool visible);
        ScreenPos ProjectWorldToScreen(const RE::NiPoint3& worldPos);

        LedgeInfo m_currentLedge;
        ScreenPos m_screenPos;
        float m_maxReach = 150.0f;
        float m_ledgeDepthThreshold = 20.0f;

        // Climbing state
        ClimbState m_climbState = ClimbState::Idle;
        RE::NiPoint3 m_startPos;
        RE::NiPoint3 m_targetPos;
        float m_interpolationTimer = 0.0f;
        static constexpr float kClimbDuration = 0.3f; // seconds
    };
}
