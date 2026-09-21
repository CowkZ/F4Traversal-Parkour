#include "hotkey.h"

#include "debug.h"
#include "ledge_detector.h"

#include <chrono>
#include <mutex>
#include <thread>

// Windows.h so aqui, isolado, para os macros dele nao atrapalharem o resto.
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#    define NOMINMAX
#endif
#include <Windows.h>

namespace
{
    constexpr int kDebugKey = VK_F8;
    constexpr int kToggleKey = VK_F7; // Tecla para ligar/desligar o detector

    bool GameHasFocus()
    {
        const auto wnd = ::GetForegroundWindow();
        if (!wnd) return false;
        DWORD pid = 0;
        ::GetWindowThreadProcessId(wnd, &pid);
        return pid == ::GetCurrentProcessId();
    }

    void PollLoop()
    {
        bool wasDown = false;
        bool wasSpaceDown = false;
        bool wasProjDown = false;
        bool wasToggleDown = false;

        for (;;) {
            const bool gameFocused = GameHasFocus();
            const bool down = gameFocused && (::GetAsyncKeyState(kDebugKey) & 0x8000) != 0;
            const bool spaceDown = gameFocused && (::GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
            const bool projDown = gameFocused && (::GetAsyncKeyState(VK_F6) & 0x8000) != 0;
            const bool toggleDown = gameFocused && (::GetAsyncKeyState(kToggleKey) & 0x8000) != 0;

            if (down && !wasDown) {
                if (const auto task = F4SE::GetTaskInterface()) {
                    task->AddTask([] {
                        Debug::PlayerSnapshot();
                        Debug::TestTraversalDetection();
                    });
                }
            }

            if (toggleDown && !wasToggleDown) {
                if (const auto task = F4SE::GetTaskInterface()) {
                    task->AddTask([] {
                        Traversal::LedgeDetector::GetSingleton()->ToggleEnabled();
                    });
                }
            }

            if (projDown && !wasProjDown) {
                if (const auto task = F4SE::GetTaskInterface()) {
                    task->AddTask([] {
                        Traversal::LedgeDetector::GetSingleton()->ToggleProjection();
                    });
                }
            }

            if (spaceDown && !wasSpaceDown) {
                if (const auto task = F4SE::GetTaskInterface()) {
                    task->AddTask([] {
                        Traversal::LedgeDetector::GetSingleton()->RequestClimb();
                    });
                }
            }

            wasDown = down;
            wasSpaceDown = spaceDown;
            wasProjDown = projDown;
            wasToggleDown = toggleDown;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    }
}

namespace Hotkey
{
    void Start()
    {
        static std::once_flag once;
        std::call_once(once, [] {
            std::thread(PollLoop).detach();
            REX::INFO("hotkey de debug ativo (F8)");
        });
    }
}
