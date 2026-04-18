#include "chat_hook.h"

#include <windows.h>
#include <cstdio>
#include <cstdint>

#include "../../third_party/MinHook.h"
#include "../features/emoji_replace.h"

struct CGameNetwork;
struct CGameNetFormAdmin;

static constexpr uintptr_t RVA_OnChatReceived = 0x0021D250;
static constexpr uintptr_t OFF_ChatTexts = 0x88;

struct RawBuf
{
    int count;
    void* data;
    int capacity;
};

struct RawChatLine
{
    int len;
    wchar_t* text;
};

using tOnChatReceived =
void(__thiscall*)(CGameNetwork* self, CGameNetFormAdmin* formAdmin, int param2);

static tOnChatReceived gOrigOnChatReceived = nullptr;
static void* gTargetOnChatReceived = nullptr;

static bool gMinHookInitialized = false;
static bool gHookInstalled = false;

static int       gLastLineLen = -1;
static uintptr_t gLastTextPtr = 0;
static wchar_t   gLastPrefix[16] = {};

static uintptr_t GetExeBase()
{
    return reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
}

static bool SafeWideReadable(const wchar_t* s, int len)
{
    if (!s || len <= 0 || len > 2048)
        return false;

    __try
    {
        volatile wchar_t first = s[0];
        volatile wchar_t last = s[len - 1];
        (void)first;
        (void)last;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

static bool SamePrefix(const wchar_t* a, const wchar_t* b, int n)
{
    for (int i = 0; i < n; ++i)
    {
        if (a[i] != b[i])
            return false;
        if (a[i] == L'\0')
            return true;
    }
    return true;
}

static void PrintWideLine(const wchar_t* ws, int len)
{
    if (!ws || len <= 0)
        return;

    char out[8192];
    const int written = WideCharToMultiByte(
        CP_UTF8, 0, ws, len, out, sizeof(out) - 1, nullptr, nullptr);

    if (written <= 0)
        return;

    out[written] = '\0';

    for (int i = 0; i < written; ++i)
    {
        if (out[i] == '\r' || out[i] == '\n')
            out[i] = ' ';
    }

    std::printf("[CHAT] %s\n", out);
    std::fflush(stdout);
}

static void TryPrintLatestChat(CGameNetwork* self, int param2)
{
    if (!self)
        return;

    auto* buf = reinterpret_cast<RawBuf*>(reinterpret_cast<uintptr_t>(self) + OFF_ChatTexts);
    if (!buf || buf->count <= 0 || !buf->data)
        return;

    auto* line = reinterpret_cast<RawChatLine*>(buf->data);

    std::printf(
        "[CHATBUF] self=0x%08X data=0x%08X count=%d cap=%d p2=%d len=%d text=0x%08X\n",
        static_cast<unsigned>(reinterpret_cast<uintptr_t>(self)),
        static_cast<unsigned>(reinterpret_cast<uintptr_t>(buf->data)),
        buf->count,
        buf->capacity,
        param2,
        line->len,
        static_cast<unsigned>(reinterpret_cast<uintptr_t>(line->text))
    );
    std::fflush(stdout);

    if (!SafeWideReadable(line->text, line->len))
        return;

    wchar_t prefix[16] = {};
    const int copyCount = (line->len < 15) ? line->len : 15;
    for (int i = 0; i < copyCount; ++i)
        prefix[i] = line->text[i];
    prefix[copyCount] = L'\0';

    if (gLastLineLen == line->len &&
        gLastTextPtr == reinterpret_cast<uintptr_t>(line->text) &&
        SamePrefix(gLastPrefix, prefix, 16))
    {
        return;
    }

    gLastLineLen = line->len;
    gLastTextPtr = reinterpret_cast<uintptr_t>(line->text);
    for (int i = 0; i < 16; ++i)
        gLastPrefix[i] = prefix[i];

    PrintWideLine(line->text, line->len);

    int newLen = line->len;
    if (ReplaceAllTokens(line->text, newLen, 2048))
    {
        line->len = newLen;
        std::printf("[MOD] emoji replacement applied\n");
        std::fflush(stdout);

        PrintWideLine(line->text, line->len);
    }
}

static void __fastcall Hooked_OnChatReceived(
    CGameNetwork* self,
    void*,
    CGameNetFormAdmin* formAdmin,
    int param2)
{
    if (gOrigOnChatReceived)
        gOrigOnChatReceived(self, formAdmin, param2);

    TryPrintLatestChat(self, param2);
}

bool ChatHook_Install()
{
    const uintptr_t base = GetExeBase();
    gTargetOnChatReceived = reinterpret_cast<void*>(base + RVA_OnChatReceived);

    std::printf("[+] EXE base        = 0x%08X\n", static_cast<unsigned>(base));
    std::printf("[+] OnChatReceived = 0x%08X\n",
        static_cast<unsigned>(reinterpret_cast<uintptr_t>(gTargetOnChatReceived)));
    std::fflush(stdout);

    MH_STATUS st = MH_Initialize();
    if (st != MH_OK)
    {
        std::printf("[-] MH_Initialize failed: %s\n", MH_StatusToString(st));
        std::fflush(stdout);
        return false;
    }

    gMinHookInitialized = true;

    st = MH_CreateHook(
        gTargetOnChatReceived,
        reinterpret_cast<LPVOID>(&Hooked_OnChatReceived),
        reinterpret_cast<LPVOID*>(&gOrigOnChatReceived));

    if (st != MH_OK)
    {
        std::printf("[-] MH_CreateHook failed: %s\n", MH_StatusToString(st));
        std::fflush(stdout);

        MH_Uninitialize();
        gMinHookInitialized = false;
        return false;
    }

    st = MH_EnableHook(gTargetOnChatReceived);
    if (st != MH_OK)
    {
        std::printf("[-] MH_EnableHook failed: %s\n", MH_StatusToString(st));
        std::fflush(stdout);

        MH_RemoveHook(gTargetOnChatReceived);
        MH_Uninitialize();
        gMinHookInitialized = false;
        return false;
    }

    gHookInstalled = true;

    std::printf("[+] Hook installed\n");
    std::fflush(stdout);
    return true;
}

void ChatHook_Remove()
{
    if (gHookInstalled && gTargetOnChatReceived)
    {
        MH_DisableHook(gTargetOnChatReceived);
        MH_RemoveHook(gTargetOnChatReceived);
        gHookInstalled = false;
    }

    if (gMinHookInitialized)
    {
        MH_Uninitialize();
        gMinHookInitialized = false;
    }

    gOrigOnChatReceived = nullptr;
    gTargetOnChatReceived = nullptr;
}