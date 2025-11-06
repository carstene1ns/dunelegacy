
#include <switch.h>
#include "Switch/input.h"
#include "DataTypes.h" // for SettingsClass

extern SettingsClass settings; // from globals.h
extern SDL_Window* window; // from sand.h
extern SDL_Renderer* renderer;

#ifdef DEBUG
    #define DBGPRINT 1
#else
    #define DBGPRINT 0
#endif
#define dbgprintf(fmt, ...) do { if (DBGPRINT) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

namespace {
    // used to convert user-friendly pointer speed values into more useable ones
    constexpr float CONTROLLER_SPEED_MOD = 1800000.0f;
    // bigger value correndsponds to faster pointer movement speed with bigger stick axis values
    constexpr float CONTROLLER_AXIS_SPEEDUP = 1.03f;

    constexpr int CONTROLLER_AXIS_DEADZONE = 3000;

    SDL_GameController* gameController = nullptr;
    std::vector<uint8_t> keyStates;
    bool emuMousePressed = false;

    float cursorSpeedup = 1.0f;

    int16_t controllerLeftXAxis = 0;
    int16_t controllerLeftYAxis = 0;
    uint32_t lastControllerTime = 0;

    std::tuple<int, int, bool> getMouseState() {
        // get mouse position inside scaled window
        int x, y;
        float logX, logY;
        uint32_t btns = SDL_GetMouseState(&x, &y);
        SDL_RenderWindowToLogical(renderer, x, y, &logX, &logY);

        return std::make_tuple(static_cast<int>(round(logX)), static_cast<int>(round(logY)),
            (btns & SDL_BUTTON(1)) || emuMousePressed);
    }

    void pushKeyEvent(const SDL_Keycode key, const bool pressed) {
        SDL_Event ev;
        SDL_Scancode sc = SDL_GetScancodeFromKey(key);
        if (pressed) {
            ev.type = SDL_KEYDOWN;
            ev.key.state = SDL_PRESSED;
        } else {
            ev.type = SDL_KEYUP;
            ev.key.state = SDL_RELEASED;
        }
        ev.key.keysym.mod = SDL_GetModState();
        ev.key.keysym.scancode = sc;
        ev.key.keysym.sym = key;
        SDL_PushEvent(&ev);

        keyStates[sc] = pressed ? 1 : 0;
    }

    void pushMouseButtonEvent(const uint8_t btn, const bool pressed) {
        auto [x, y, ignored] = getMouseState();

        SDL_Event ev;
        ev.type = pressed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
        ev.button.button = btn;
        ev.button.x = x;
        ev.button.y = y;
        SDL_PushEvent(&ev);
        emuMousePressed = pressed && (btn == SDL_BUTTON_LEFT);
    }
}

namespace Switch::Input {
    float scrollFactorRight = 1.0f;
    float scrollFactorLeft = 1.0f;
    float scrollFactorUp = 1.0f;
    float scrollFactorDown = 1.0f;

    void HandleControllerAxisEvent(const SDL_ControllerAxisEvent& motion);
    void HandleControllerButtonEvent(const SDL_ControllerButtonEvent& button);

    void Start() {
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

        // Emulated keyboard
        int numkeys;
        SDL_GetKeyboardState(&numkeys);
        keyStates.resize(numkeys, 0);

        // Open Controller
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                dbgprintf("Opening Joystick %d as Controller\n", i);
                gameController = SDL_GameControllerOpen(i);
                if(gameController != nullptr)
                    return;
            }
        }
    }

    void Stop() {
        // Close Controller
        if (SDL_GameControllerGetAttached(gameController)) {
            dbgprintf("Closing Controller\n");
            SDL_GameControllerClose(gameController);
            gameController = nullptr;
        }

        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    void Update() {
        // Left Controller Axis to Mouse Motion
        const uint32_t currentTime = SDL_GetTicks();
        const double deltaTime = currentTime - lastControllerTime;
        lastControllerTime = currentTime;

        if (controllerLeftXAxis != 0 || controllerLeftYAxis != 0) {
            const int16_t xSign = (controllerLeftXAxis > 0) - (controllerLeftXAxis < 0);
            const int16_t ySign = (controllerLeftYAxis > 0) - (controllerLeftYAxis < 0);

            int x, y;
            SDL_GetMouseState(&x, &y);

            x += std::pow(std::abs(controllerLeftXAxis), CONTROLLER_AXIS_SPEEDUP) * xSign * deltaTime
                * settings.general.mouseSpeed / CONTROLLER_SPEED_MOD * cursorSpeedup;
            y += std::pow(std::abs(controllerLeftYAxis), CONTROLLER_AXIS_SPEEDUP) * ySign * deltaTime
                * settings.general.mouseSpeed / CONTROLLER_SPEED_MOD * cursorSpeedup;

            SDL_WarpMouseInWindow(window, x, y);
        }
    }

    bool ProcessEvent(const SDL_Event event) {
        switch (event.type) {
            case SDL_CONTROLLERDEVICEREMOVED:
                if (gameController != nullptr) {
                    const SDL_GameController* removedController = SDL_GameControllerFromInstanceID(event.jdevice.which);
                    if (removedController == gameController) {
                        dbgprintf("Closing Controller\n");
                        SDL_GameControllerClose(gameController);
                        gameController = nullptr;
                    }
                }
                break;
            case SDL_CONTROLLERDEVICEADDED:
                if (gameController == nullptr) {
                    dbgprintf("Opening Controller\n");
                    gameController = SDL_GameControllerOpen(event.jdevice.which);
                }
                break;

            case SDL_CONTROLLERAXISMOTION:
                HandleControllerAxisEvent(event.caxis);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                HandleControllerButtonEvent(event.cbutton);
                break;

            default:
                return false; // event unhandled
                break;
        }

        return true; // event handled
    }

    void HandleControllerAxisEvent(const SDL_ControllerAxisEvent& motion) {
        auto normalize = [](int value) -> float {
            if (!value)
                return 0;

            if(value > 0)
                return value / static_cast<float>(SDL_JOYSTICK_AXIS_MAX);

            return value / static_cast<float>(SDL_JOYSTICK_AXIS_MIN);
        };

        switch (motion.axis) {
            case SDL_CONTROLLER_AXIS_LEFTX:
                if (std::abs(motion.value) > CONTROLLER_AXIS_DEADZONE)
                    controllerLeftXAxis = motion.value;
                else
                    controllerLeftXAxis = 0;
                break;

            case SDL_CONTROLLER_AXIS_LEFTY:
                if (std::abs(motion.value) > CONTROLLER_AXIS_DEADZONE)
                    controllerLeftYAxis = motion.value;
                else
                    controllerLeftYAxis = 0;
                break;

            case SDL_CONTROLLER_AXIS_RIGHTX:
                if (motion.value > CONTROLLER_AXIS_DEADZONE) {
                    pushKeyEvent(SDLK_RIGHT, true);
                    scrollFactorRight = normalize(motion.value);
                } else {
                    pushKeyEvent(SDLK_RIGHT, false);
                    scrollFactorRight = 1.0f;
                }

                if (motion.value < -CONTROLLER_AXIS_DEADZONE) {
                    pushKeyEvent(SDLK_LEFT, true);
                    scrollFactorLeft = normalize(motion.value);
                } else {
                    pushKeyEvent(SDLK_LEFT, false);
                    scrollFactorLeft = 1.0f;
                }
                break;

            case SDL_CONTROLLER_AXIS_RIGHTY:
                if (motion.value > CONTROLLER_AXIS_DEADZONE) {
                    pushKeyEvent(SDLK_DOWN, true);
                    scrollFactorDown = normalize(motion.value);
                } else {
                    pushKeyEvent(SDLK_DOWN, false);
                    scrollFactorDown = 1.0f;
                }

                if (motion.value < -CONTROLLER_AXIS_DEADZONE) {
                    pushKeyEvent(SDLK_UP, true);
                    scrollFactorUp = normalize(motion.value);
                } else {
                    pushKeyEvent(SDLK_UP, false);
                    scrollFactorUp = 1.0f;
                }
                break;

            case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                pushKeyEvent(SDLK_g, motion.value == SDL_JOYSTICK_AXIS_MAX);
                break;
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                pushKeyEvent(SDLK_f, motion.value == SDL_JOYSTICK_AXIS_MAX);
                break;

            default:
                // axis unhandled
                break;
        }
    }

    void HandleControllerButtonEvent(const SDL_ControllerButtonEvent& button) {
        bool pressed = (button.type == SDL_CONTROLLERBUTTONDOWN);
        SDL_Keymod modState = SDL_GetModState();

        switch (button.button) {
        case SDL_CONTROLLER_BUTTON_B: // A
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            pushMouseButtonEvent(SDL_BUTTON_LEFT, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_A: // B
            pushMouseButtonEvent(SDL_BUTTON_RIGHT, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_X: // Y
            pushKeyEvent(SDLK_a, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_Y: // X
            pushKeyEvent(SDLK_s, pressed);
            break;

        case SDL_CONTROLLER_BUTTON_START: // +
            pushKeyEvent(SDLK_ESCAPE, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_BACK: // -
            pushKeyEvent(SDLK_SPACE, pressed);
            break;

        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            SDL_SetModState(static_cast<SDL_Keymod>(pressed ? (modState | KMOD_CTRL) : (modState & ~KMOD_CTRL)));
            pushKeyEvent(SDLK_LCTRL, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            SDL_SetModState(static_cast<SDL_Keymod>(pressed ? (modState | KMOD_SHIFT) : (modState & ~KMOD_SHIFT)));
            pushKeyEvent(SDLK_RSHIFT, pressed);
            break;

        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            cursorSpeedup = pressed ? 2.0f : 1.0f;
            break;

        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            pushKeyEvent(SDLK_1, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            pushKeyEvent(SDLK_2, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            pushKeyEvent(SDLK_3, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            pushKeyEvent(SDLK_4, pressed);
            break;

        default:
            break;
        }
    }

    const uint8_t* GetKeyboardState(int *numkeys) {
        if(numkeys)
            *numkeys = keyStates.size();

        return keyStates.data();
    }

    void DrawCursor() {
        auto [x, y, clicked] = getMouseState();

        #define COLOR_DARK SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200)
        #define COLOR_LIGHT SDL_SetRenderDrawColor(renderer, 185, 152, 130, 255)

        // save draw color
        Uint8 r, g, b, a;
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

        // draw mouse cursor outer cross
        if (clicked) COLOR_LIGHT; else COLOR_DARK;
        SDL_Rect rectO[2] = {{x - 11, y - 1, 22, 3},
                        {x - 1, y - 11, 3, 22}};
        SDL_RenderDrawRects(renderer, &rectO[0], 2);
        
        // inner cross
        if (clicked) COLOR_DARK; else COLOR_LIGHT;
        SDL_Rect rectI[2] = {{x - 10, y, 20, 1},
                        {x, y - 10, 1, 20}};
        SDL_RenderFillRects(renderer, &rectI[0], 2);

        // restore color
        SDL_SetRenderDrawColor(renderer, r, g, b, a);

        #undef COLOR_DARK
        #undef COLOR_LIGHT
    }
}
