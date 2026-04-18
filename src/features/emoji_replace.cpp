#include "emoji_replace.h"
#include "emoji_list.h"

#include <cwchar>
#include <cstring>
#include <cwctype>

static bool WideStartsWithInsensitive(const wchar_t* text, const wchar_t* token, int tokenLen)
{
    for (int i = 0; i < tokenLen; ++i)
    {
        const wchar_t a = text[i];
        const wchar_t b = token[i];

        if (towlower(a) != towlower(b))
            return false;
    }
    return true;
}

bool ReplaceAllTokens(wchar_t* ws, int& len, int maxCapacityChars)
{
    if (!ws || len <= 0 || maxCapacityChars <= 0)
        return false;

    bool changed = false;
    int i = 0;

    while (i < len)
    {
        bool matched = false;

        for (int m = 0; m < kEmojiCount; ++m)
        {
            const wchar_t* tok = kEmojis[m].token;
            const wchar_t* rep = kEmojis[m].replacement;

            const int tokLen = static_cast<int>(wcslen(tok));
            const int repLen = static_cast<int>(wcslen(rep));

            if (i + tokLen > len)
                continue;

            if (!WideStartsWithInsensitive(ws + i, tok, tokLen))
                continue;

            const int newLen = len - tokLen + repLen;
            if (newLen >= maxCapacityChars)
                continue;

            const int tailSrcIndex = i + tokLen;
            const int tailDstIndex = i + repLen;
            const int tailCount = len - tailSrcIndex + 1; // +1 pour le '\0'

            std::memmove(
                ws + tailDstIndex,
                ws + tailSrcIndex,
                static_cast<size_t>(tailCount) * sizeof(wchar_t)
            );

            for (int j = 0; j < repLen; ++j)
                ws[i + j] = rep[j];

            len = newLen;
            changed = true;
            matched = true;
            i += repLen;
            break;
        }

        if (!matched)
            ++i;
    }

    return changed;
}