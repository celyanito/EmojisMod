#pragma once

struct CGameNetwork;

struct RawChatLine
{
    int len;
    wchar_t* text;
};

namespace chat_anim
{
    void SetNetwork(CGameNetwork* net);
    void TrackRawLine(RawChatLine* line);
    void Tick();
    void Clear();
}