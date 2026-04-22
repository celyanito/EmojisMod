# Reverse notes — TMUF chat

## Key function

- `CGameNetwork::OnChatReceived`
  - stable MinHook hook on the current build
  - ideal entry point to capture received messages before final local display

## Confirmed chat buffer

Inside `CGameNetwork`:

- `field_0x88` = chat text history
- Observed buffer layout:

```cpp
struct RawBuf {
    int count;
    void* data;
    int capacity;
};Likely chat line layout

The data pointer inside the buffer points to an array of 8-byte entries observed as:

struct RawChatLine {
    int len;
    wchar_t* text;
};

Observed behavior:

index 0 = most recent message
when a new message arrives, history is inserted at the front
History limit

After insertion, the game reduces the size of buffers 0x88 / 0x94 / 0xA0 to a limit read around field_0xB0.
In practice: ~40 lines max.

What worked
Hooking OnChatReceived
Reading the chat buffer after calling the original function
Reading wide strings (wchar_t*)
Local in-place modification of the received text
Replacing tokens like :wave: even in the middle of a message
What caused problems before
Manual 5-byte inline hook: unstable because of prologue / SEH / security cookie issues
Attempting to use GetUtf8 in this path: crashes on this build
Anti-duplication based only on count and data: false positives when the buffer stabilizes
Recommended direction
Stay on OnChatReceived
Modify wchar_t* text in place after the original returns
Use case-insensitive matching
For replacements longer than the token, implement a proper reallocation or secondary-buffer strategy
