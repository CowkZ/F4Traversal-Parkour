#include "hotkey.h"

#include "debug.h"
#include "ledge_detector.h"

namespace
{
    const char* MessageName(std::uint32_t a_type) noexcept;
    void UpdateTask();
    void UpdateLoop();

    const char* MessageName(std::uint32_t a_type) noexcept
    {
        using M = F4SE::MessagingInterface;
        switch (a_type) {
        case M::kPostLoad:      return "PostLoad";
        case M::kPostPostLoad:  return "PostPostLoad";
        case M::kPreLoadGame:   return "PreLoadGame";
        case M::kPostLoadGame:  return "PostLoadGame";
        case M::kPreSaveGame:   return "PreSaveGame";
        case M::kPostSaveGame:  return "PostSaveGame";
        case M::kDeleteGame:    return "DeleteGame";
        case M::kInputLoaded:   return "InputLoaded";
        case M::kNewGame:       return "NewGame";
        case M::kGameLoaded:    return "GameLoaded";
        case M::kGameDataReady: return "GameDataReady";
        default:                return "?";
        }
    }

    bool g_updateTaskScheduled = false;

    void UpdateTask()
    {
        g_updateTaskScheduled = false;
        if (auto* detector = Traversal::LedgeDetector::GetSingleton())
        {
            detector->Update();
        }
        // Reschedule for next update
        UpdateLoop();
    }

    void UpdateLoop()
    {
        if (g_updateTaskScheduled) {
            return; // already scheduled
        }
        if (const auto task = F4SE::GetTaskInterface()) {
            g_updateTaskScheduled = true;
            task->AddTask(UpdateTask);
        }
    }
}

    void F4SEAPI OnF4SEMessage(F4SE::MessagingInterface::Message* a_msg)
    {
        if (!a_msg) {
            return;
        }

        REX::INFO("mensagem F4SE: {} ({})", MessageName(a_msg->type), a_msg->type);

        if (a_msg->type == F4SE::MessagingInterface::kGameDataReady) {
            Hotkey::Start();
            UpdateLoop(); // start the update loop
        }
    }
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
    F4SE::Init(a_f4se);

    REX::INFO("F4Traversal carregado");

    const auto messaging = F4SE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnF4SEMessage)) {
        REX::ERROR("falha ao registrar listener de mensagens do F4SE");
        return false;
    }

    return true;
}