
#ifndef _SWITCH_INPUT_H
#define _SWITCH_INPUT_H

#include <SDL.h>

namespace Switch::Input {
    constexpr int MOUSESPEED_MIN = 10;
    constexpr int MOUSESPEED_MAX = 80;

    extern float scrollFactorRight, scrollFactorLeft, scrollFactorUp, scrollFactorDown;

    void Start();
    void Stop();
    void Update();
    bool ProcessEvent(const SDL_Event event);
    const uint8_t* GetKeyboardState(int *numkeys);
    void DrawCursor();
}

#endif
