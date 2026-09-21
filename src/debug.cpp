#include "debug.h"

#include <numbers>
#include <format>

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

        // 1. Pegar a posição da câmera (olhos do jogador)
        const auto camera = RE::PlayerCamera::GetSingleton();
        if (!camera) return;

        NiPoint3 startPos;
        if (!camera->GetCameraPosition(startPos, true)) return;

        // 2. Calcular o Forward Vector a partir da rotação da câmera
        const auto state = camera->GetCameraCurrentState();
        if (!state) return;

        NiQuaternion rot;
        state->GetRotation(rot);

        // Converter Quaternion para Forward Vector (Z-Forward no F4)
        NiPoint3 forwardVec;
        forwardVec.x = 2.0f * (rot.x * rot.z + rot.w * rot.y);
        forwardVec.y = 2.0f * (rot.y * rot.z - rot.w * rot.x);
        forwardVec.z = 1.0f - 2.0f * (rot.x * rot.x + rot.y * rot.y);

        // 3. Definir o alcance do Raycast (ex: 1.5 metros)
        const float range = 150.0f;
        NiPoint3 endPos;
        endPos.x = startPos.x + (forwardVec.x * range);
        endPos.y = startPos.y + (forwardVec.y * range);
        endPos.z = startPos.z + (forwardVec.z * range);

        // 4. Executar o Raycast usando a engine do jogo
        // NOTA: BGSInterface::GetRaycast não existe no CommonLibF4.
        // Temporariamente simulamos a detecção para validar a posição e direção via HUD.
        bool hit = false;

        if (hit) {
            // Implementação real de GetHitObject() virá aqui após mapear a função de raycast do F4
            RE::SendHUDMessage::ShowHUDMessage("[✋] Objeto Detectado!", "", false, false);
        } else {
            // Feedback na HUD para validar que a função está rodando e os vetores estão corretos
            std::string debugInfo = std::format(
                "Raycast Debug:\nPos: {:.1f}, {:.1f}, {:.1f}\nDir: {:.2f}, {:.2f}, {:.2f}",
                startPos.x, startPos.y, startPos.z, forwardVec.x, forwardVec.y, forwardVec.z);

            RE::SendHUDMessage::ShowHUDMessage(debugInfo.c_str(), "", false, false);
            REX::INFO("TraversalRaycast: Simulando Raycast. Pos=({:.1f}, {:.1f}, {:.1f}) Dir=({:.2f}, {:.2f}, {:.2f})",
                startPos.x, startPos.y, startPos.z, forwardVec.x, forwardVec.y, forwardVec.z);
        }
    }
}
