#include "tmuf.h"
#include "memory.h"
#include "offsets.h"

namespace emj
{
    uintptr_t get_trackmania()
    {
        uintptr_t base = mem::exe_base();
        uintptr_t app = 0;

        if (!mem::safe_read(base + offsets::O_CTRACKMANIA, app))
            return 0;

        return app;
    }

    uintptr_t get_network()
    {
        uintptr_t tmapp = get_trackmania();
        if (!mem::is_probably_valid_ptr(tmapp))
            return 0;

        uintptr_t network = 0;
        if (!mem::safe_read(tmapp + offsets::O_NETWORK, network))
            return 0;

        return network;
    }

    uintptr_t get_profile()
    {
        uintptr_t tmapp = get_trackmania();
        if (!mem::is_probably_valid_ptr(tmapp))
            return 0;

        uintptr_t profile = 0;
        if (!mem::safe_read(tmapp + offsets::O_PROFILE, profile))
            return 0;

        return profile;
    }

    uintptr_t get_menu_manager()
    {
        uintptr_t tmapp = get_trackmania();
        if (!mem::is_probably_valid_ptr(tmapp))
            return 0;

        uintptr_t menu = 0;
        if (!mem::safe_read(tmapp + offsets::O_MENUMGR, menu))
            return 0;

        return menu;
    }

    bool is_game_ready()
    {
        return get_trackmania() != 0;
    }
}