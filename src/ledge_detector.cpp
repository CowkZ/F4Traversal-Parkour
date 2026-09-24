#include "ledge_detector.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <RE/M/Main.h>

namespace
{
    bool IsOnMainThread()
    {
        auto* main = RE::Main::GetSingleton();
        if (!main) {
            return false;
        }
        auto currentThreadId = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(Scaleform::GetCurrentThreadId()));
        return main->threadID == currentThreadId;
    }
}

namespace Traversal
{
    namespace
    {
        std::atomic<bool> g_gameReady{ false };
        std::atomic<bool> g_enabled{ false };  // detector comeca DESLIGADO (F7 liga)
        std::atomic<bool> g_climbing{ false };

        // ---- parametros de calibragem (unidades do jogo; jogador ~128 de altura) ----
        constexpr float kStartOffset = 22.0F;                // raios horizontais comecam a frente da capsula do jogador
        constexpr float kKneeHeight = 40.0F;                 // altura (dos pes) do raio que procura a parede
        constexpr float kReach = 75.0F;                      // alcance do raio da parede, a partir do offset
        constexpr float kMinLedge = 45.0F;                   // abaixo disso e degrau, nao escalada
        constexpr float kMaxLedge = 190.0F;                  // acima disso o jogador nao alcanca
        constexpr float kTallProbe = kMaxLedge + 40.0F;      // altura do raio "parede alta demais"
        constexpr float kOverTop = 12.0F;                    // quanto alem da parede procuramos o topo
        constexpr float kHeadroom = 100.0F;                  // espaco livre exigido acima do topo
        constexpr float kLowMax = 90.0F;
        constexpr float kMediumMax = 150.0F;
        constexpr float kClimbForward = 15.0F;               // quanto avancar sobre o topo ao terminar
        constexpr float kClimbDuration = 0.35F;              // segundos
        constexpr float kRisePhase = 0.6F;                   // fracao do tempo subindo na vertical
        constexpr int   kDebounceTicks = 3;
        constexpr auto  kMessageCooldown = std::chrono::milliseconds(1500);

        // Camada de colisao usada nos raios
        constexpr std::uint32_t kRayFilter = static_cast<std::uint32_t>(RE::COL_LAYER::kCharController);

        struct RayHit
        {
            bool         hit = false;
            float        fraction = 1.0F;
            RE::NiPoint3 point{};
            RE::NiPoint3 normal{};
            bool         hasNormal = false;
        };

        // Raycast via TESObjectCELL::Pick (API Verificada no activity_log)
        RayHit CastRay(RE::TESObjectCELL* a_cell, const RE::NiPoint3& a_from, const RE::NiPoint3& a_to)
        {
            RE::bhkPickData pd;
            pd.castQuery.m_filterData.m_collisionFilterInfo = kRayFilter;
            pd.SetStartEnd(a_from, a_to);
            static_cast<void>(a_cell->Pick(pd));

            RayHit out;
            if (!pd.HasHit()) {
                return out;
            }

            out.hit = true;
            out.fraction = pd.GetHitFraction();
            out.point = RE::NiPoint3{
                a_from.x + (a_to.x - a_from.x) * out.fraction,
                a_from.y + (a_to.y - a_from.y) * out.fraction,
                a_from.z + (a_to.z - a_from.z) * out.fraction
            };

            const auto& n = pd.result.normal;
            out.normal = RE::NiPoint3{ n.x, n.y, n.z };
            out.hasNormal = (n.x * n.x + n.y * n.y + n.z * n.z) > 0.25F;
            return out;
        }

        RE::NiPoint3 Forward(const RE::PlayerCharacter* a_player)
        {
            // Game object access must happen on main thread
            if (!IsOnMainThread()) {
                return RE::NiPoint3{ 0.0F, 0.0F, 0.0F };
            }

            const float yaw = a_player->data.angle.z;
            return RE::NiPoint3{ std::sin(yaw), std::cos(yaw), 0.0F };
        }

        bool IsUiBlocking()
        {
            // UI access must happen on main thread
            if (!IsOnMainThread()) {
                return true; // Assume UI is blocking to be safe
            }

            const auto ui = RE::UI::GetSingleton();
            if (!ui) return true;
            return ui->GetMenuOpen<RE::MainMenu>() ||
                   ui->GetMenuOpen<RE::LoadingMenu>() ||
                   ui->GetMenuOpen<RE::PauseMenu>() ||
                   ui->GetMenuOpen<RE::PipboyMenu>() ||
                   ui->GetMenuOpen<RE::Console>();
        }

        const char* KindName(LedgeKind a_kind)
        {
            switch (a_kind) {
            case LedgeKind::Low:    return "baixa";
            case LedgeKind::Medium: return "media";
            case LedgeKind::High:   return "alta";
            default:                return "nenhuma";
            }
        }
    }

    LedgeDetector* LedgeDetector::GetSingleton()
    {
        static LedgeDetector singleton;
        return &singleton;
    }

    void LedgeDetector::SetGameReady(bool a_ready) noexcept { g_gameReady.store(a_ready); }
    bool LedgeDetector::IsGameReady() noexcept { return g_gameReady.load(); }
    bool LedgeDetector::IsEnabled() noexcept { return g_enabled.load(); }
    bool LedgeDetector::IsClimbing() noexcept { return g_climbing.load(); }

    void LedgeDetector::ResetState()
    {
        m_currentLedge = LedgeInfo{};
        m_screenPos = ScreenPos{};
        m_lastEval = Eval{};
        m_candidateKind = LedgeKind::None;
        m_shownKind = LedgeKind::None;
        m_candidateTicks = 0;
        m_climbState = ClimbState::Idle;
        g_climbing.store(false);
    }

    void LedgeDetector::ToggleEnabled()
    {
        // Member variable access should happen on main thread for consistency
        if (!IsOnMainThread()) {
            return;
        }

        const bool now = !g_enabled.load();
        ResetState();
        g_enabled.store(now);
        REX::INFO("detector de bordas: {}", now ? "LIGADO" : "desligado");
        RE::SendHUDMessage::ShowHUDMessage(
            now ? "F4Traversal: detector LIGADO" : "F4Traversal: desligado", "", false, false);
    }

    std::string LedgeDetector::DescribeLast() const
    {
        return std::format("detector {} | {} | altura {:.0f}", 
            g_enabled.load() ? "ligado" : "desligado", m_lastEval.reason, m_lastEval.height);
    }

    LedgeDetector::Eval LedgeDetector::Evaluate(RE::PlayerCharacter* a_player)
    {
        // Game object access must happen on main thread
        if (!IsOnMainThread()) {
            Eval out;
            out.reason = "not on main thread";
            return out;
        }

        Eval out;
        const auto cell = a_player->parentCell;
        if (!cell) { out.reason = "sem celula"; return out; }

        const RE::NiPoint3 pos = a_player->GetPosition();
        const RE::NiPoint3 fwd = Forward(a_player);

        const float ox = pos.x + fwd.x * kStartOffset;
        const float oy = pos.y + fwd.y * kStartOffset;

        const RE::NiPoint3 wallFrom{ ox, oy, pos.z + kKneeHeight };
        const RE::NiPoint3 wallTo{ ox + fwd.x * kReach, oy + fwd.y * kReach, pos.z + kKneeHeight };
        const RayHit wall = CastRay(cell, wallFrom, wallTo);
        if (!wall.hit) { out.reason = "sem parede"; return out; }
        if (wall.hasNormal && std::fabs(wall.normal.z) > 0.5F) { out.reason = "superficie nao vertical"; return out; }
        const float wallDist = kReach * wall.fraction;

        const float tallReach = wallDist + 30.0F;
        const RE::NiPoint3 tallFrom{ ox, oy, pos.z + kTallProbe };
        const RE::NiPoint3 tallTo{ ox + fwd.x * tallReach, oy + fwd.y * tallReach, pos.z + kTallProbe };
        if (CastRay(cell, tallFrom, tallTo).hit) { out.reason = "parede alta demais"; return out; }

        const float px = wall.point.x + fwd.x * kOverTop;
        const float py = wall.point.y + fwd.y * kOverTop;
        const RayHit top = CastRay(cell, RE::NiPoint3{ px, py, pos.z + kTallProbe }, RE::NiPoint3{ px, py, pos.z + kKneeHeight });
        if (!top.hit) { out.reason = "sem topo"; return out; }

        const float topZ = top.point.z;
        const float height = topZ - pos.z;
        out.height = height;

        if (height < kMinLedge) { out.reason = "baixo demais (degrau)"; return out; }
        if (height > kMaxLedge) { out.reason = "alto demais"; return out; }
        if (top.hasNormal && top.normal.z < 0.7F) { out.reason = "topo inclinado"; return out; }

        const RayHit head = CastRay(cell, RE::NiPoint3{ px, py, topZ + 4.0F }, RE::NiPoint3{ px, py, topZ + 4.0F + kHeadroom });
        if (head.hit) { out.reason = "sem espaco livre em cima"; return out; }

        out.top = RE::NiPoint3{ px, py, topZ };
        out.kind = height < kLowMax ? LedgeKind::Low : (height < kMediumMax ? LedgeKind::Medium : LedgeKind::High);
        out.reason = "borda valida";
        return out;
    }

    void LedgeDetector::ApplyResult(const Eval& a_eval)
    {
        m_lastEval = a_eval;
        if (a_eval.kind == m_candidateKind) {
            m_candidateTicks = std::min(m_candidateTicks + 1, kDebounceTicks);
        } else {
            m_candidateKind = a_eval.kind;
            m_candidateTicks = 1;
        }

        const auto fill = [&] {
            m_currentLedge.position = a_eval.top;
            m_currentLedge.height = a_eval.height;
            m_currentLedge.kind = a_eval.kind;
            m_currentLedge.isValid = true;
        };

        if (m_candidateTicks >= kDebounceTicks && m_candidateKind != m_shownKind) {
            m_shownKind = m_candidateKind;
            if (m_shownKind == LedgeKind::None) {
                m_currentLedge = LedgeInfo{};
                UpdateMarker(RE::NiPoint3{}, false);
                REX::INFO("borda perdida ({})", a_eval.reason);
                return;
            }
            fill();
            UpdateMarker(a_eval.top, true);
            REX::INFO("borda {}: altura={:.0f} topo=({:.0f}, {:.0f}, {:.0f})", KindName(m_shownKind), a_eval.height, a_eval.top.x, a_eval.top.y, a_eval.top.z);
            const auto now = std::chrono::steady_clock::now();
            if (now - m_lastMessage >= kMessageCooldown) {
                m_lastMessage = now;
                const auto text = std::format("[MAO] Escalavel ({}) - altura {:.0f}", KindName(m_shownKind), a_eval.height);
                RE::SendHUDMessage::ShowHUDMessage(text.c_str(), "", false, false);
            }
        } else if (m_shownKind != LedgeKind::None && a_eval.kind != LedgeKind::None) {
            fill();
        }
    }

    void LedgeDetector::Update()
    {
        // Game object access must happen on main thread
        if (!IsOnMainThread()) {
            return;
        }

        if (!g_enabled.load()) {
            if (m_climbState != ClimbState::Idle) ResetState();
            return;
        }
        if (!g_gameReady.load()) return;
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !player->loadedData || !player->parentCell || IsUiBlocking()) return;
        if (m_climbState == ClimbState::Interpolating) {
            TickClimb(player);
            return;
        }
        ApplyResult(Evaluate(player));
    }

    void LedgeDetector::TickClimb(RE::PlayerCharacter* a_player)
    {
        // Game object access must happen on main thread
        if (!IsOnMainThread()) {
            return;
        }

        // Abort if game not ready or UI blocked
        if (!g_gameReady.load() || IsUiBlocking()) {
            ResetState();
            return;
        }

        const float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_climbStart).count();
        const float t = std::clamp(elapsed / kClimbDuration, 0.0F, 1.0F);
        RE::NiPoint3 p{};
        if (t < kRisePhase) {
            const float k = t / kRisePhase;
            p = RE::NiPoint3{ m_startPos.x, m_startPos.y, m_startPos.z + (m_targetPos.z - m_startPos.z) * k };
        } else {
            const float k = (t - kRisePhase) / (1.0F - kRisePhase);
            p = RE::NiPoint3{ m_startPos.x + (m_targetPos.x - m_startPos.x) * k, m_startPos.y + (m_targetPos.y - m_startPos.y) * k, m_targetPos.z };
        }
        a_player->SetPosition(p, true);
        if (t >= 1.0F) {
            REX::INFO("escalada concluida");
            m_climbState = ClimbState::Idle;
            g_climbing.store(false);
            m_currentLedge = LedgeInfo{};
            m_shownKind = LedgeKind::None;
            m_candidateKind = LedgeKind::None;
            m_candidateTicks = 0;
        }
    }

    void LedgeDetector::UpdateMarker(const RE::NiPoint3& a_pos, bool a_visible)
    {
        // Member variable access should happen on main thread for consistency
        if (!IsOnMainThread()) {
            return;
        }

        if (m_projectionEnabled) {
            m_screenPos = ProjectWorldToScreen(a_pos);
            m_screenPos.visible = a_visible && m_screenPos.visible;
        } else {
            m_screenPos = ScreenPos{};
        }
    }

    ScreenPos LedgeDetector::ProjectWorldToScreen(const RE::NiPoint3& a_pos)
    {
        // Game object access must happen on main thread
        if (!IsOnMainThread()) {
            return ScreenPos{};
        }

        ScreenPos out{};
        const auto camera = RE::Main::WorldRootCamera();
        if (!camera) return out;
        float x = 0.0f, y = 0.0f, z = 0.0f;
        if (camera->WorldPtToScreenPt3(a_pos, x, y, z, 10.0f)) {
            out.x = x; out.y = y; out.visible = true;
            REX::INFO("PROJECAO: ponto=({:.0f}, {:.0f}, {:.0f}) -> screen=({:.3f}, {:.3f}, {:.3f}) visible={}",
                a_pos.x, a_pos.y, a_pos.z, x, y, z, out.visible);
        }
        return out;
    }

    void LedgeDetector::ToggleProjection()
    {
        // Member variable access should happen on main thread for consistency
        if (!IsOnMainThread()) {
            return;
        }

        m_projectionEnabled = !m_projectionEnabled;
        REX::INFO("projecao 2D: {}", m_projectionEnabled ? "LIGADA" : "desligada");
    }

    void LedgeDetector::RequestClimb()
    {
        // Game object access must happen on main thread
        if (!IsOnMainThread()) {
            return;
        }

        if (!g_enabled.load() || !g_gameReady.load() || m_climbState != ClimbState::Idle) return;
        if (!m_currentLedge.isValid) {
            REX::INFO("escalada pedida, mas nao ha borda valida");
            return;
        }
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !player->loadedData || !player->parentCell || IsUiBlocking()) return;
        const RE::NiPoint3 fwd = Forward(player);
        m_startPos = player->GetPosition();
        m_targetPos = RE::NiPoint3{ m_currentLedge.position.x + fwd.x * kClimbForward, m_currentLedge.position.y + fwd.y * kClimbForward, m_currentLedge.position.z };
        m_climbStart = std::chrono::steady_clock::now();
        m_climbState = ClimbState::Interpolating;
        g_climbing.store(true);
        REX::INFO("escalada iniciada: de ({:.0f}, {:.0f}, {:.0f}) para ({:.0f}, {:.0f}, {:.0f})", m_startPos.x, m_startPos.y, m_startPos.z, m_targetPos.x, m_targetPos.y, m_targetPos.z);
    }
    }

} // namespace Traversal
