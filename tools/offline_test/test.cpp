#include <thread>
#include "ledge_detector.h"
#include <cstdio>
using namespace Traversal;

static int fails = 0;
static void check(bool ok, const char* what) { std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }

struct Scenario { const char* name; std::vector<RE::Box> world; float yaw; };

static LedgeKind run(const Scenario& sc, float& height, const char*& reason) {
    RE::g_world = sc.world;
    auto* p = RE::PlayerCharacter::GetSingleton();
    static RE::TESObjectCELL cell; static RE::LOADED_REF_DATA ld;
    p->parentCell = &cell; p->loadedData = &ld; p->pos = { 0, 0, 0 }; p->data.angle.z = sc.yaw;
    auto* d = LedgeDetector::GetSingleton();
    if (!LedgeDetector::IsEnabled()) d->ToggleEnabled();
    d->ToggleEnabled(); d->ToggleEnabled();   // reseta o estado (desliga/liga)
    LedgeDetector::SetGameReady(true);
    for (int i = 0; i < 5; ++i) d->Update();
    auto& l = d->GetCurrentLedge();
    height = l.height; reason = nullptr;
    std::printf("  -> %s\n", d->DescribeLast().c_str());
    return l.isValid ? l.kind : LedgeKind::None;
}

int main() {
    float h; const char* r;
    const float PI = 3.14159265F;

    std::printf("A) muro/borda a 100 de altura, jogador olhando +Y (yaw 0)\n");
    { auto k = run({ "A", { { -100, 60, 0, 100, 200, 100 } }, 0.0F }, h, r);
      check(k == LedgeKind::Medium, "classificou como media"); check(std::fabs(h - 100.0F) < 1.0F, "altura ~100"); 
      auto& l = LedgeDetector::GetSingleton()->GetCurrentLedge();
      check(std::fabs(l.position.z - 100.0F) < 1.0F, "ponto do topo em z=100"); }

    std::printf("B) mesma borda, jogador olhando +X (yaw = pi/2), muro ao longo de +X\n");
    { auto k = run({ "B", { { 60, -100, 0, 200, 100, 100 } }, PI / 2 }, h, r);
      check(k == LedgeKind::Medium, "detectou girando o yaw (sin,cos coerente)"); }

    std::printf("C) muro alto demais (topo a 300)\n");
    { auto k = run({ "C", { { -100, 60, 0, 100, 200, 300 } }, 0.0F }, h, r); check(k == LedgeKind::None, "rejeitou"); }

    std::printf("D) degrau baixo (topo a 30) - abaixo do joelho\n");
    { auto k = run({ "D", { { -100, 60, 0, 100, 200, 30 } }, 0.0F }, h, r); check(k == LedgeKind::None, "rejeitou"); }

    std::printf("E) borda a 170 com viga a 250 (menos de 100 de espaco em cima)\n");
    { auto k = run({ "E", { { -100, 60, 0, 100, 200, 170 }, { -100, 60, 250, 100, 200, 260 } }, 0.0F }, h, r); check(k == LedgeKind::None, "rejeitou por falta de espaco"); }

    std::printf("F) borda baixa a 60 -> baixa; borda a 170 -> alta\n");
    { auto k1 = run({ "F1", { { -100, 60, 0, 100, 200, 60 } }, 0.0F }, h, r); check(k1 == LedgeKind::Low, "60 = baixa");
      auto k2 = run({ "F2", { { -100, 60, 0, 100, 200, 170 } }, 0.0F }, h, r); check(k2 == LedgeKind::High, "170 = alta"); }

    std::printf("G) sem nada a frente\n");
    { auto k = run({ "G", {}, 0.0F }, h, r); check(k == LedgeKind::None, "nenhuma borda"); }

    std::printf("H) escalada: da borda A ate o topo\n");
    { run({ "H", { { -100, 60, 0, 100, 200, 100 } }, 0.0F }, h, r);
      auto* d = LedgeDetector::GetSingleton(); auto* p = RE::PlayerCharacter::GetSingleton();
      d->RequestClimb(); check(LedgeDetector::IsClimbing(), "escalada iniciou");
      for (int i = 0; i < 40 && LedgeDetector::IsClimbing(); ++i) { d->Update(); std::this_thread::sleep_for(std::chrono::milliseconds(16)); }
      std::printf("  posicao final: (%.0f, %.0f, %.0f)\n", p->pos.x, p->pos.y, p->pos.z);
      check(!LedgeDetector::IsClimbing(), "escalada terminou"); check(std::fabs(p->pos.z - 102.0F) < 1.0F, "terminou em cima (z ~ 102)"); check(p->pos.y > 60.0F, "avancou para cima da borda"); }

    std::printf("\n%s (%d falhas)\n", fails ? "COM FALHAS" : "TUDO OK", fails);
    return fails ? 1 : 0;
}
