#include "chat_anim.h"

#include <Windows.h>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cwchar>
#include <cstdint>
#include <cstring>

namespace
{
    struct RawBuf
    {
        int count;
        RawChatLine* data;
        int capacity;
    };

    struct TrackedLine
    {
        int lastFrame = -1;
    };

    static constexpr std::uintptr_t OFF_ChatRaw = 0x88;
    static constexpr std::uintptr_t OFF_ChatCache = 0xA0;
    static constexpr std::uintptr_t OFF_ChatDirty = 0xAC;

    static constexpr int kMaxChatCapacity = 2048;
    static constexpr ULONGLONG kFrameMs = 100;

    static const wchar_t* kBirdToken = L":bird:";
    static const wchar_t* kBirdFrames[] =
    {
        L"$fff注$z$s", // frame 1
        L"$fff册$z$s", // frame 2
        L"$fff使$z$s", // frame 3
        L"$fff尚$z$s", // frame 4
        L"$fff帐$z$s", // frame 5
    };

    static CGameNetwork* gNet = nullptr;
    static std::mutex gMutex;
    static std::unordered_map<std::uintptr_t, TrackedLine> gTracked;

    static wchar_t LowerAscii(wchar_t c)
    {
        if (c >= L'A' && c <= L'Z')
            return static_cast<wchar_t>(c - L'A' + L'a');
        return c;
    }

    static bool StartsWithInsensitive(const wchar_t* text, const wchar_t* token, int tokenLen)
    {
        for (int i = 0; i < tokenLen; ++i)
        {
            if (LowerAscii(text[i]) != LowerAscii(token[i]))
                return false;
        }
        return true;
    }

    static bool StartsWithExact(const wchar_t* text, const wchar_t* token, int tokenLen)
    {
        for (int i = 0; i < tokenLen; ++i)
        {
            if (text[i] != token[i])
                return false;
        }
        return true;
    }

    static bool ContainsBirdToken(const wchar_t* text, int len)
    {
        if (!text || len <= 0)
            return false;

        const int tokenLen = static_cast<int>(std::wcslen(kBirdToken));
        if (tokenLen <= 0 || tokenLen > len)
            return false;

        for (int i = 0; i <= len - tokenLen; ++i)
        {
            if (StartsWithInsensitive(text + i, kBirdToken, tokenLen))
                return true;
        }

        return false;
    }

    static bool ReplacePatternAt(RawChatLine* line, int pos, int srcLen, const wchar_t* dst)
    {
        if (!line || !line->text || !dst)
            return false;

        const int dstLen = static_cast<int>(std::wcslen(dst));
        const int newLen = line->len - srcLen + dstLen;

        if (newLen <= 0 || newLen >= kMaxChatCapacity)
            return false;

        const int tailSrcIndex = pos + srcLen;
        const int tailDstIndex = pos + dstLen;
        const int tailCount = line->len - tailSrcIndex + 1; // +1 pour le '\0'

        std::memmove(
            line->text + tailDstIndex,
            line->text + tailSrcIndex,
            static_cast<size_t>(tailCount) * sizeof(wchar_t)
        );

        for (int i = 0; i < dstLen; ++i)
            line->text[pos + i] = dst[i];

        line->len = newLen;
        return true;
    }

    static bool ReplaceAllBirdTokensAndFramesInPlace(RawChatLine* line, const wchar_t* targetFrame)
    {
        if (!line || !line->text || !targetFrame)
            return false;

        if (line->len <= 0 || line->len >= kMaxChatCapacity)
            return false;

        const int tokenLen = static_cast<int>(std::wcslen(kBirdToken));
        bool matchedAnything = false;

        int i = 0;
        while (i < line->len)
        {
            bool matched = false;

            if (i + tokenLen <= line->len &&
                StartsWithInsensitive(line->text + i, kBirdToken, tokenLen))
            {
                if (!ReplacePatternAt(line, i, tokenLen, targetFrame))
                    return matchedAnything;

                matchedAnything = true;
                matched = true;
                i += static_cast<int>(std::wcslen(targetFrame));
            }
            else
            {
                for (int f = 0; f < static_cast<int>(sizeof(kBirdFrames) / sizeof(kBirdFrames[0])); ++f)
                {
                    const wchar_t* srcFrame = kBirdFrames[f];
                    const int srcLen = static_cast<int>(std::wcslen(srcFrame));

                    if (i + srcLen > line->len)
                        continue;

                    if (!StartsWithExact(line->text + i, srcFrame, srcLen))
                        continue;

                    if (!ReplacePatternAt(line, i, srcLen, targetFrame))
                        return matchedAnything;

                    matchedAnything = true;
                    matched = true;
                    i += static_cast<int>(std::wcslen(targetFrame));
                    break;
                }
            }

            if (!matched)
                ++i;
        }

        return matchedAnything;
    }
}

namespace chat_anim
{
    void SetNetwork(CGameNetwork* net)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        gNet = net;
    }

    void TrackRawLine(RawChatLine* line)
    {
        if (!line || !line->text)
            return;

        if (line->len <= 0 || line->len >= kMaxChatCapacity)
            return;

        if (!ContainsBirdToken(line->text, line->len))
            return;

        std::lock_guard<std::mutex> lock(gMutex);
        gTracked[reinterpret_cast<std::uintptr_t>(line->text)] = TrackedLine{};
    }

    void Tick()
    {
        CGameNetwork* net = nullptr;
        {
            std::lock_guard<std::mutex> lock(gMutex);
            net = gNet;
        }

        if (!net)
            return;

        auto* rawBuf = reinterpret_cast<RawBuf*>(reinterpret_cast<std::uintptr_t>(net) + OFF_ChatRaw);
        auto* cacheBuf = reinterpret_cast<RawBuf*>(reinterpret_cast<std::uintptr_t>(net) + OFF_ChatCache);
        auto* dirtyPtr = reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(net) + OFF_ChatDirty);

        if (!rawBuf || !cacheBuf || !dirtyPtr)
            return;

        if (rawBuf->count <= 0 || !rawBuf->data)
            return;

        if (cacheBuf->count <= 0 || !cacheBuf->data)
            return;

        const int frameIndex = static_cast<int>(
            (GetTickCount64() / kFrameMs) % (sizeof(kBirdFrames) / sizeof(kBirdFrames[0]))
            );

        std::lock_guard<std::mutex> lock(gMutex);

        std::vector<std::uintptr_t> liveKeys;
        liveKeys.reserve(rawBuf->count);

        const int count = (rawBuf->count < cacheBuf->count) ? rawBuf->count : cacheBuf->count;
        bool anyUiChanged = false;

        for (int i = 0; i < count; ++i)
        {
            RawChatLine* rawLine = &rawBuf->data[i];
            RawChatLine* cacheLine = &cacheBuf->data[i];

            if (!rawLine || !rawLine->text)
                continue;

            const std::uintptr_t key = reinterpret_cast<std::uintptr_t>(rawLine->text);
            auto it = gTracked.find(key);
            if (it == gTracked.end())
                continue;

            liveKeys.push_back(key);

            if (it->second.lastFrame == frameIndex)
                continue;

            if (ReplaceAllBirdTokensAndFramesInPlace(cacheLine, kBirdFrames[frameIndex]))
            {
                it->second.lastFrame = frameIndex;
                anyUiChanged = true;
            }
        }

        for (auto it = gTracked.begin(); it != gTracked.end(); )
        {
            bool stillLive = false;

            for (std::uintptr_t key : liveKeys)
            {
                if (key == it->first)
                {
                    stillLive = true;
                    break;
                }
            }

            if (!stillLive)
                it = gTracked.erase(it);
            else
                ++it;
        }

        if (anyUiChanged)
            *dirtyPtr = 1;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(gMutex);
        gTracked.clear();
        gNet = nullptr;
    }
}