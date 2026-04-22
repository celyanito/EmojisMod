#pragma once

bool TopIcon_Initialize();
void TopIcon_Tick();
void TopIcon_Shutdown();

void TopIcon_SetLiveParent(void* parentLayout, void* sourceWidget);
void TopIcon_ClearLiveParent();