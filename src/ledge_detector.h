#pragma once

// Os tipos RE:: vem do pch.h (forcado pelo xmake). Nada de includes de RE/ aqui.

#include <atomic>
#include <chrono>
#include <string>

namespace Traversal
{
    enum class ClimbState
    {
        Idle,
        Interpolating,
        OnLedge
    };

    enum class LedgeKind
    {
        None,
        Low,     // ate ~90 unidades
        Medium,  // ate ~150
        High     // ate ~190
    };

    struct LedgeInfo
    {
        RE::NiPoint3 position{};   // ponto no topo da borda (superficie onde o jogador vai ficar)
        float        height = 0.0F;  // altura do topo em relacao aos pes do jogador
        LedgeKind    kind = LedgeKind::None;
        bool         isValid = false;
    };

    struct ScreenPos
    {
        float x = 0.0F;
        float y = 0.0F;
        bool  visible = false;
    };

    class LedgeDetector
    {
    public:
        static LedgeDetector* GetSingleton();

        // Flags globais: podem ser lidas/escritas de qualquer thread.
        static void SetGameReady(bool a_ready) noexcept;
        static bool IsGameReady() noexcept;
        static bool IsEnabled() noexcept;
        static bool IsClimbing() noexcept;

        // SOMENTE na thread do jogo (via F4SE::GetTaskInterface()->AddTask).
        void ToggleEnabled();
        void ToggleProjection();
        void Update();
        void RequestClimb();
        std::string DescribeLast() const;

        const LedgeInfo& GetCurrentLedge() const { return m_currentLedge; }
        ScreenPos        GetLedgeScreenPos() const { return m_screenPos; }

    private:
        LedgeDetector() = default;

        struct Eval
        {
            LedgeKind    kind = LedgeKind::None;
            RE::NiPoint3 top{};
            float        height = 0.0F;
            const char*  reason = "-";
        };

        Eval         Evaluate(RE::PlayerCharacter* a_player);
        void         ApplyResult(const Eval& a_eval);
        void         TickClimb(RE::PlayerCharacter* a_player);
        void         ResetState();
        void         UpdateMarker(const RE::NiPoint3& a_pos, bool a_visible);
        ScreenPos    ProjectWorldToScreen(const RE::NiPoint3& a_pos);

        LedgeInfo m_currentLedge;
        ScreenPos m_screenPos;
        bool        m_projectionEnabled = false;
        Eval        m_lastEval;

        // debounce do indicador
        LedgeKind m_candidateKind = LedgeKind::None;
        LedgeKind m_shownKind = LedgeKind::None;
        int       m_candidateTicks = 0;
        std::chrono::steady_clock::time_point m_lastMessage{};

        // escalada
        ClimbState   m_climbState = ClimbState::Idle;
        RE::NiPoint3 m_startPos{};
        RE::NiPoint3 m_targetPos{};
        std::chrono::steady_clock::time_point m_climbStart{};
    };
}
