# TMUF Chat Emoji Mod — reverse notes + prototype

This repository is the working base for a TMUF mod that intercepts received chat messages, reads them locally, and replaces tokens such as `:wave:` with font-pack codes (for example `$fff回`) in order to modify local display only.

## Confirmed current state

- Stable MinHook hook on `CGameNetwork::OnChatReceived`.
- The chat history buffer is located at offset `0x88` inside `CGameNetwork`.
- Confirmed buffer layout:
  - `int count`
  - `void* data`
  - `int capacity`
- The chat lines observed in `field_0x88` behave like 8-byte records:
  - `int len`
  - `wchar_t* text`
- The most recent message is stored at index `0`, which matches `InsertNewElemAt(..., 0)`.
- The local history limit is around 40 lines (`field_0xB0`-related logic).
- In-place modification of `wchar_t* text` after `OnChatReceived` returns works locally.
- Replacements also work in the middle of a sentence.
- Matching should be case-insensitive.

## Files

- `dllmain.cpp`: DLL prototype with hook, chat reading, and local emote replacement.
- `REVERSE_NOTES.md`: summary of the useful reverse-engineering findings.
- `EMOTES_MAP.md`: token -> font-pack code mapping.
- `TODO.md`: next steps.
- `.gitignore`: standard Visual Studio / MSBuild ignores.

## Dependency

This prototype assumes MinHook is included in the project (vendored in the repository or copied into the Visual Studio project).

## Short-term goal

- Stabilize repeated replacements.
- Expand the emote table.
- Properly handle replacements that increase the string length.
