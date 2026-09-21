#pragma once
#include "stub.h"
#define F4SEAPI
namespace F4SE {
    struct LoadInterface {};
    inline void Init(const LoadInterface*) {}
    class MessagingInterface {
    public:
        enum : std::uint32_t { kPostLoad, kPostPostLoad, kPreLoadGame, kPostLoadGame, kPreSaveGame, kPostSaveGame, kDeleteGame, kInputLoaded, kNewGame, kGameLoaded, kGameDataReady };
        struct Message { std::uint32_t type; };
        using EventCallback = void(Message*);
        bool RegisterListener(EventCallback*) const { return true; }
    };
    class TaskInterface { public: void AddTask(std::function<void()> f) const { f(); } };
    inline const MessagingInterface* GetMessagingInterface() { static MessagingInterface m; return &m; }
    inline const TaskInterface* GetTaskInterface() { static TaskInterface t; return &t; }
}
#define F4SE_PLUGIN_LOAD(x) extern "C" bool F4SEPlugin_Load(x)
