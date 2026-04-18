#include <windows.h>
#include "app.h"

__declspec(dllexport)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, AppMainThread, nullptr, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        AppShutdown();
        break;
    }

    return TRUE;
}