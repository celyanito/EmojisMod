#pragma once
#include <cstdint>

namespace emj
{
    uintptr_t get_trackmania();
    uintptr_t get_network();
    uintptr_t get_profile();
    uintptr_t get_menu_manager();

    bool is_game_ready();
}