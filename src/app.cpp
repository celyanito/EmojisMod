#include "app.h"

#include <windows.h>
#include <cstdio>

#include "hooks/chat_hook.h"

static bool gConsoleReady = false;

static void SetupConsole()
{
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
}

DWORD WINAPI AppMainThread(LPVOID)
{
    SetupConsole();

    if (!ChatHook_Install())
    {
        std::printf("[-] ChatHook_Install failed\n");
        std::fflush(stdout);
        return 0;
    }

    std::printf("[*] Join a server and wait for chat lines...\n");
    std::fflush(stdout);
    return 0;
}

void AppShutdown()
{
    ChatHook_Remove();

    if (gConsoleReady)
    {
        FreeConsole();
        gConsoleReady = false;
    }
}