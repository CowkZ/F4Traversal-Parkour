#include "debug.h"

#include <numbers>
#include <format>
#include <RE/N/NiPoint3.h>
#include <RE/N/NiQuaternion.h>

namespace Debug
{
    void PlayerSnapshot()
    {
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            REX::WARN("PlayerSnapshot: jogador indisponivel (ainda no menu?)");
            return;
        }

        const auto  pos = player->GetPosition();
        const float yawDeg = player->data.angle.z * (180.0F / std::numbers::pi_v<float>);
        const auto  cell = player->GetParentCell();
        const auto  cellID = cell ? cell->GetFormID() : 0u;

        REX::INFO(
            "snapshot: pos=({:.1f}, {:.1f}, {:.1f}) yaw={:.1f} celula={:08X}",
            pos.x, pos.y, pos.z, yawDeg, cellID);

        const auto text = std::format(
            "F4Traversal: X {:.0f}  Y {:.0f}  Z {:.0f}  yaw {:.0f}",
            pos.x, pos.y, pos.z, yawDeg);
        RE::SendHUDMessage::ShowHUDMessage(text.c_str(), "", false, false);
    }

    void TestTraversalDetection()
    {
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        const auto camera = RE::PlayerCamera::GetSingleton();
        if (!camera) return;

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

        const float range = 150.0f;
        RE::NiPoint3 endPos;
        endPos.x = startPos.x + (forwardVec.x * range);
        endPos.y = startPos.y + (forwardVec.y * range);
        endPos.z = startPos.z + (forwardVec.z * range);

        bool hit = false;

        if (hit) {
            RE::SendHUDMessage::ShowHUDMessage("[✋] Objeto Detectado!", "", false, false);
        } else {
            std::string debugInfo = std::format(
                "Raycast Debug:\nPos: {:.1f}, {:.1f}, {:.1f}\nDir: {:.2f}, {:.2f}, {:.2f}",
                startPos.x, startPos.y, startPos.z, forwardVec.x, forwardVec.y, forwardVec.z);

            RE::SendHUDMessage::ShowHUDMessage(debugInfo.c_str(), "", false, false);
            REX::INFO("TraversalRaycast: Simulando Raycast. Pos=({:.1f}, {:.1f}, {:.1f}) Dir=({:.2f}, {:.2f}, {:.2f})",
                startPos.x, startPos.y, startPos.z, forwardVec.x, forwardVec.y, forwardVec.z);
        }
    }
}
