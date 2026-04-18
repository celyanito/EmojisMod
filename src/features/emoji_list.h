#pragma once

struct EmojiMap
{
    const wchar_t* token;
    const wchar_t* replacement;
};

extern const EmojiMap kEmojis[];
extern const int kEmojiCount;