#include "top_icon.h"
#include "debug_file_log.h"

#include <Windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace
{
    constexpr wchar_t kIconPath[] = L"Media\\EmojisModIcon.png";

    struct PreparedPath
    {
        std::uintptr_t a;
        std::uintptr_t b;
    };

    template <typename T>
    static T ResolveOrdinal(WORD ordinal)
    {
        HMODULE exe = GetModuleHandleW(nullptr);
        if (!exe)
            return nullptr;

        return reinterpret_cast<T>(GetProcAddress(exe, MAKEINTRESOURCEA(ordinal)));
    }

    using FnPreparePathMaybe = void(__thiscall*)(PreparedPath* outPath, const wchar_t* path);
    using FnLoadImageResourceMaybe = int(__thiscall*)(void* loadCtx, void* basePathCtx, PreparedPath* preparedPath, int flags);
    using FnFinalizeLoadStepMaybe = void(__thiscall*)(PreparedPath* preparedPath);
    using FnRegisterIconsResourceMaybe = int(__stdcall*)(void* outHandlePtr, int imageResource, int kind);

    struct Api
    {
        FnPreparePathMaybe           preparePath = nullptr;             // ordinal 18
        FnLoadImageResourceMaybe     loadImageResource = nullptr;       // ordinal 650
        FnFinalizeLoadStepMaybe      finalizeLoadStep = nullptr;        // ordinal 12874
        FnRegisterIconsResourceMaybe registerIconsResource = nullptr;   // ordinal 590

        void* loadCtxExportAddr = nullptr;                              // ordinal 57482 export address
    };

    Api gApi{};

    bool  gInitialized = false;
    bool  gFaulted = false;
    bool  gLoggedWaiting = false;
    bool  gLoggedLiveParent = false;
    bool  gAtlasAttempted = false;

    void* gLiveTopParent = nullptr;
    void* gLiveSourceWidget = nullptr;
    void* gIconAtlas = nullptr;

    static bool ResolveApi()
    {
        if (gApi.preparePath)
            return true;

        gApi.preparePath = ResolveOrdinal<FnPreparePathMaybe>(18);
        gApi.loadImageResource = ResolveOrdinal<FnLoadImageResourceMaybe>(650);
        gApi.finalizeLoadStep = ResolveOrdinal<FnFinalizeLoadStepMaybe>(12874);
        gApi.registerIconsResource = ResolveOrdinal<FnRegisterIconsResourceMaybe>(590);
        gApi.loadCtxExportAddr = ResolveOrdinal<void*>(57482);

        const bool ok =
            gApi.preparePath &&
            gApi.loadImageResource &&
            gApi.finalizeLoadStep &&
            gApi.registerIconsResource &&
            gApi.loadCtxExportAddr;

        if (!ok)
        {
            DebugFileLog("[-] TopIcon: failed to resolve required ordinals");
            return false;
        }

        DebugFileLog("[TopIcon] required ordinals resolved");
        return true;
    }

    static void* GetLoadContext()
    {
        if (!gApi.loadCtxExportAddr)
            return nullptr;

        return *reinterpret_cast<void**>(gApi.loadCtxExportAddr);
    }

    static bool LoadAtlasFromPng()
    {
        if (gIconAtlas)
            return true;

        void* loadCtx = GetLoadContext();
        if (!loadCtx)
        {
            DebugFileLog("[-] TopIcon: load context is null");
            return false;
        }

        PreparedPath prepared{};
        gApi.preparePath(&prepared, kIconPath);

        const int imageResource = gApi.loadImageResource(loadCtx, nullptr, &prepared, 0);
        gApi.finalizeLoadStep(&prepared);

        if (!imageResource)
        {
            DebugFileLog("[-] TopIcon: LoadImageResourceMaybe failed for %S", kIconPath);
            return false;
        }

        void* atlasHandle = nullptr;
        const int regOk = gApi.registerIconsResource(&atlasHandle, imageResource, 7);
        if (!regOk || !atlasHandle)
        {
            DebugFileLog("[-] TopIcon: RegisterIconsResourceMaybe failed");
            return false;
        }

        gIconAtlas = atlasHandle;
        DebugFileLog("[TopIcon] atlas loaded from %S -> %p", kIconPath, atlasHandle);
        return true;
    }
}

void TopIcon_SetLiveParent(void* parentLayout, void* sourceWidget)
{
    gLiveTopParent = parentLayout;
    gLiveSourceWidget = sourceWidget;
    gLoggedLiveParent = false;

    DebugFileLog(
        "[TopIcon] live parent captured: parent=%p sourceWidget=%p",
        parentLayout,
        sourceWidget
    );
}

void TopIcon_ClearLiveParent()
{
    gLiveTopParent = nullptr;
    gLiveSourceWidget = nullptr;
    gLoggedLiveParent = false;

    DebugFileLog("[TopIcon] live parent cleared");
}

bool TopIcon_Initialize()
{
    if (gInitialized)
        return true;

    if (!ResolveApi())
        return false;

    gInitialized = true;
    DebugFileLog("[TopIcon] initialized");
    return true;
}

void TopIcon_Tick()
{
    if (gFaulted)
        return;

    long stage = 0;

    __try
    {
        stage = 1;
        if (!gInitialized)
        {
            if (!TopIcon_Initialize())
                return;
        }

        stage = 2;
        if (!gLiveTopParent)
        {
            if (!gLoggedWaiting)
            {
                gLoggedWaiting = true;
                DebugFileLog("[TopIcon] waiting for a live parent from TMUnlimiter hook");
            }
            return;
        }

        stage = 3;
        if (!gLoggedLiveParent)
        {
            gLoggedLiveParent = true;
            DebugFileLog(
                "[TopIcon] live parent is ready: parent=%p sourceWidget=%p",
                gLiveTopParent,
                gLiveSourceWidget
            );
        }

        stage = 4;
        if (!gAtlasAttempted)
        {
            gAtlasAttempted = true;
            DebugFileLog("[TopIcon] safe build: atlas sanity check only");
            (void)LoadAtlasFromPng();
        }

        stage = 5;
        // Intentionally no UI creation here yet.
        // No fake context.
        // No parent reconstruction.
        // No button attach.
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        gFaulted = true;
        DebugFileLog("[-] TopIcon: exception inside TopIcon_Tick stage %ld", stage);
    }
}

void TopIcon_Shutdown()
{
    gInitialized = false;
    gFaulted = false;
    gLoggedWaiting = false;
    gLoggedLiveParent = false;
    gAtlasAttempted = false;

    gLiveTopParent = nullptr;
    gLiveSourceWidget = nullptr;
    gIconAtlas = nullptr;

    std::memset(&gApi, 0, sizeof(gApi));

    DebugFileLog("[TopIcon] shutdown");
}