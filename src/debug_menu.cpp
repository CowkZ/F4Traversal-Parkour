#include "debug_menu.h"
#include "ledge_detector.h"
#include <RE/F4SE/F4SE.h>
#include <RE/H/hknpWorld.h>
#include <format>
#include <string>

// O F4SEMenuFramework usa ImGui internamente
#include <imgui.h>
#include <RE/F4SEMenuFramework/F4SEMenuFramework.h>

namespace Traversal
{
    void DebugMenu::Initialize()
    {
        // Registra a janela de debug no framework
        // A função Render será chamada pelo framework sempre que o menu estiver aberto
        F4SEMenuFramework::AddWindow([] {
            Render();
        });
    }

    void DebugMenu::Render()
    {
        auto detector = LedgeDetector::GetSingleton();
        
        ImGui::Begin("F4Traversal Debug");

        ImGui::Text("Status do Sistema");
        ImGui::Separator();

        bool enabled = LedgeDetector::IsEnabled();
        ImGui::Text("Detector: %s", enabled ? "LIGADO" : "DESLIGADO");
        
        bool proj = detector->GetLedgeScreenPos().visible; // Simplificação para exemplo
        ImGui::Text("Projeção: %s", proj ? "ATIVA" : "INATIVA");

        ImGui::Spacing();
        ImGui::Text("Informações da Borda");
        ImGui::Separator();

        const auto& ledge = detector->GetCurrentLedge();
        if (ledge.isValid) {
            ImGui::Text("Altura: %.1f", ledge.height);
            ImGui::Text("Tipo: %s", "Desconhecido"); // Precisaria de KindName
            ImGui::Text("Pos X: %.1f", ledge.position.x);
            ImGui::Text("Pos Y: %.1f", ledge.position.y);
            ImGui::Text("Pos Z: %.1f", ledge.position.z);
        } else {
            ImGui::Text("Nenhuma borda detectada");
        }

        ImGui::End();
    }
}
