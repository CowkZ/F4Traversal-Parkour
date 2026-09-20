#include "hotkey.h"

#include "debug.h"

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
    constexpr int kDebugKey = VK_F8;  // troque a tecla aqui

    bool GameHasFocus()
    {
        const auto wnd = ::GetForegroundWindow();
        if (!wnd) {
            return false;
        }

        DWORD pid = 0;
        ::GetWindowThreadProcessId(wnd, &pid);
        return pid == ::GetCurrentProcessId();
    }

    void PollLoop()
    {
        bool wasDown = false;

        for (;;) {
            const bool down = GameHasFocus() && (::GetAsyncKeyState(kDebugKey) & 0x8000) != 0;

            if (down && !wasDown) {
                // a acao mexe em estado do jogo: precisa rodar na thread principal
                if (const auto task = F4SE::GetTaskInterface()) {
                    task->AddTask([] {
                        // Alternando entre snapshot e teste de detecção para facilitar seu teste
                        static bool toggle = false;
                        if (toggle) {
                            Debug::PlayerSnapshot();
                        } else {
                            Debug::TestTraversalDetection();
                        }
                        toggle = !toggle;
                    });
                }
            }

            wasDown = down;
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
