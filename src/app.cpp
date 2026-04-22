#include "app.h"
#include "debug/ordinal_test.h"

#include <Windows.h>
#include <cstdio>
#include <atomic>

#include "ui/top_icon.h"
#include "hooks/chat_hook.h"
#include "ui/debug_file_log.h"

static bool gConsoleReady = false;
static std::atomic<bool> gRunning{ false };

static void SetupConsole()
{
#if defined(_DEBUG)
    if (!AllocConsole())
        return;

    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONIN$", "r", stdin);
    freopen_s(&f, "CONERR$", "w", stderr);

    SetConsoleTitleA("TMUF Emoji Hook");
    std::printf("[+] Console ready\n");
    std::fflush(stdout);

    gConsoleReady = true;
#endif
}

DWORD WINAPI AppMainThread(LPVOID)
{
    SetupConsole();

#if defined(_DEBUG)
    std::printf("[BOOT] console alive\n");
    std::fflush(stdout);
    RunOrdinalExportTest();
#endif

    DebugFileLog("[BOOT] logger alive");

    if (!ChatHook_Install())
    {
#if defined(_DEBUG)
        std::printf("[-] ChatHook_Install failed\n");
        std::fflush(stdout);
#endif
        DebugFileLog("[-] ChatHook_Install failed");
        return 0;
    }

    gRunning.store(true);

#if defined(_DEBUG)
    std::printf("[*] Join a server and wait for chat lines...\n");
    std::printf("[*] UI cache tick started\n");
    std::fflush(stdout);
#endif

    DebugFileLog("[*] UI cache tick started");

    while (gRunning.load())
    {
        ChatHook_Tick();
        TopIcon_Tick();
        Sleep(30);
    }

    TopIcon_Shutdown();
    return 0;
}

void AppShutdown()
{
    gRunning.store(false);
    TopIcon_Shutdown();
    ChatHook_Remove();

#if defined(_DEBUG)
    if (gConsoleReady)
    {
        FreeConsole();
        gConsoleReady = false;
    }
#endif
}