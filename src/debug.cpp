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

        // 1. Pegar a posição da câmera (olhos do jogador)
        const auto camera = RE::PlayerCamera::GetSingleton();
        if (!camera) return;

        const auto startPos = camera->GetPosition();
        const auto forwardVec = camera->GetForwardVector();

        // 2. Definir o alcance do Raycast (ex: 1.5 metros)
        const float range = 150.0f;
        const auto endPos = startPos + (forwardVec * range);

        // 3. Executar o Raycast usando a engine do jogo
        // Note: No CommonLibF4, usamos o sistema de colisão do jogo
        auto result = RE::BGSInterface::GetRaycast(startPos, endPos);

        if (result) {
            // Pegar o objeto atingido (Ref)
            const auto object = result->GetHitObject();
            if (object) {
                const auto name = object->GetName();
                const std::string objectName = name ? name->AsString() : "Objeto Desconhecido";

                // Feedback na HUD e no Log
                RE::SendHUDMessage::ShowHUDMessage(
                    std::format("[✋] Detectado: {}", objectName).c_str(),
                    "", false, false);

                REX::INFO("TraversalRaycast: Objeto atingido -> {}", objectName);
            }
        } else {
            // Opcional: logar que nada foi detectado (desativar em produção para evitar spam)
            // REX::INFO("TraversalRaycast: Nada à frente.");
        }
    }
}
