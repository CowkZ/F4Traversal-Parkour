#include "debug.h"

#include <numbers>

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

        // Simulando a detecção: mostra a "mãozinha" na HUD para validar o feedback visual
        RE::SendHUDMessage::ShowHUDMessage("[✋] Superfície Escalável Detectada!", "", false, false);
        REX::INFO("TraversalTest: Feedback visual de escalada ativado (Simulação).");
    }
}
