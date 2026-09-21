#include "debug.h"

#include "ledge_detector.h"

#include <cmath>
#include <format>
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

    // Nao faz raycast. Serve para conferir a convencao de direcao: ande em linha reta para
    // a "frente" e compare com a posicao do snapshot anterior (a frente deve coincidir com o
    // deslocamento: frente=(sin(yaw), cos(yaw))).
    void TestTraversalDetection()
    {
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return;
        }

        const float yaw = player->data.angle.z;
        const float fx = std::sin(yaw);
        const float fy = std::cos(yaw);

        const auto detector = Traversal::LedgeDetector::GetSingleton()->DescribeLast();
        const auto text = std::format("Frente: {:.2f}, {:.2f}\n{}", fx, fy, detector);
        RE::SendHUDMessage::ShowHUDMessage(text.c_str(), "", false, false);

        REX::INFO("frente=({:.2f}, {:.2f}) | {}", fx, fy, detector);
    }
}
