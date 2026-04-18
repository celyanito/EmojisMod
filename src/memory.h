#pragma once
#include <Windows.h>
#include <cstdint>

namespace mem
{
    inline uintptr_t exe_base()
    {
        return reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    }

    template <typename T>
    inline T read(uintptr_t addr)
    {
        return *reinterpret_cast<T*>(addr);
    }

    inline bool is_probably_valid_ptr(uintptr_t p)
    {
        if (p < 0x10000)
            return false;

        MEMORY_BASIC_INFORMATION mbi = {};
        if (!VirtualQuery(reinterpret_cast<LPCVOID>(p), &mbi, sizeof(mbi)))
            return false;

        if (mbi.State != MEM_COMMIT)
            return false;

        DWORD protect = mbi.Protect & 0xFF;
        if (protect == PAGE_NOACCESS || protect == PAGE_EXECUTE)
            return false;

        return true;
    }

    template <typename T>
    inline bool safe_read(uintptr_t addr, T& out)
    {
        if (!is_probably_valid_ptr(addr))
            return false;

        __try
        {
            out = *reinterpret_cast<T*>(addr);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }
}