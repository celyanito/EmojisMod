#include "debug_file_log.h"

#include <Windows.h>
#include <cstdarg>
#include <cstdio>

static void ConsoleEcho(const char* text)
{
    if (!text)
        return;

    std::printf("%s\n", text);
    std::fflush(stdout);
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
}

void DebugFileLog(const char* fmt, ...)
{
    char buffer[2048] = {};

    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
    va_end(args);

    ConsoleEcho(buffer);

    char finalLine[2300] = {};
    _snprintf_s(finalLine, sizeof(finalLine), _TRUNCATE, "%s\r\n", buffer);

    char tempPath[MAX_PATH] = {};
    DWORD tempLen = GetTempPathA(MAX_PATH, tempPath);

    if (tempLen == 0 || tempLen >= MAX_PATH)
    {
        DWORD err = GetLastError();
        std::printf("[LOGFILE] GetTempPathA failed, err=%lu\n", err);
        std::fflush(stdout);

        lstrcpyA(tempPath, ".\\");
    }

    char filePath[MAX_PATH] = {};
    _snprintf_s(
        filePath,
        sizeof(filePath),
        _TRUNCATE,
        "%s%s",
        tempPath,
        "EmojisMod_topicon.log"
    );

    std::printf("[LOGFILE] target = %s\n", filePath);
    std::fflush(stdout);

    HANDLE hFile = CreateFileA(
        filePath,
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        std::printf("[LOGFILE] CreateFileA failed, err=%lu\n", err);
        std::fflush(stdout);
        return;
    }

    SetFilePointer(hFile, 0, nullptr, FILE_END);

    DWORD written = 0;
    DWORD len = static_cast<DWORD>(lstrlenA(finalLine));

    if (!WriteFile(hFile, finalLine, len, &written, nullptr))
    {
        DWORD err = GetLastError();
        std::printf("[LOGFILE] WriteFile failed, err=%lu\n", err);
        std::fflush(stdout);
        CloseHandle(hFile);
        return;
    }

    FlushFileBuffers(hFile);
    CloseHandle(hFile);

    std::printf("[LOGFILE] wrote %lu bytes\n", written);
    std::fflush(stdout);
}