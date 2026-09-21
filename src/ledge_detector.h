#pragma once

#include <RE/F4/PlayerCharacter.h>
#include <RE/N/NiPoint3.h>
#include <RE/N/NiQuaternion.h>
#include <RE/H/HUDMarkerData.h>
#include <optional>

namespace Traversal
{
    struct LedgeInfo
    {
        RE::NiPoint3 position;
        bool isValid = false;
    };

    class LedgeDetector
    {
    public:
        static LedgeDetector* GetSingleton();

        void Update();
        const LedgeInfo& GetCurrentLedge() const { return m_currentLedge; }

    private:
        LedgeDetector() = default;

        bool PerformRaycast(const RE::NiPoint3& start, const RE::NiPoint3& dir, float range, RE::NiPoint3& outHitPoint, RE::NiPoint3& outNormal);
        void UpdateMarker(const RE::NiPoint3& pos, bool visible);

        LedgeInfo m_currentLedge;
        float m_maxReach = 150.0f;
        float m_ledgeDepthThreshold = 20.0f;
    };
}
