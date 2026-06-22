#pragma once

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <map>
#include <queue>
#include <string>

struct GLFWwindow;
struct GLFWcursor;

namespace AriaFlow
{

enum ModifierKey : uint8_t
{
    MODIFIER_NONE  = 0b000000,
    MODIFIER_SHIFT = 0b000001,
    MODIFIER_CTRL  = 0b000010,
    MODIFIER_ALT   = 0b000100,
    MODIFIER_SUPER = 0b001000,
    MODIFIER_CAPS  = 0b010000,
    MODIFIER_NUM   = 0b100000
};

inline ModifierKey operator|(ModifierKey a, ModifierKey b)
{ return static_cast<ModifierKey>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b)); }

enum InputButton : uint16_t
{
    KEY_NONE = 0,

    KEY_ESCAPE        = 256,
    KEY_ENTER         = 257,
    KEY_TAB           = 258,
    KEY_BACKSPACE     = 259,
    KEY_INSERT        = 260,
    KEY_DELETE        = 261,
    KEY_RIGHT         = 262,
    KEY_LEFT          = 263,
    KEY_DOWN          = 264,
    KEY_UP            = 265,
    KEY_PAGE_UP       = 266,
    KEY_PAGE_DOWN     = 267,
    KEY_HOME          = 268,
    KEY_END           = 269,
    KEY_CAPS_LOCK     = 280,
    KEY_SCROLL_LOCK   = 281,
    KEY_NUM_LOCK      = 282,
    KEY_PRINT_SCREEN  = 283,
    KEY_PAUSE         = 284,
    KEY_F1            = 290,
    KEY_F2            = 291,
    KEY_F3            = 292,
    KEY_F4            = 293,
    KEY_F5            = 294,
    KEY_F6            = 295,
    KEY_F7            = 296,
    KEY_F8            = 297,
    KEY_F9            = 298,
    KEY_F10           = 299,
    KEY_F11           = 300,
    KEY_F12           = 301,
    KEY_F13           = 302,
    KEY_F14           = 303,
    KEY_F15           = 304,
    KEY_F16           = 305,
    KEY_F17           = 306,
    KEY_F18           = 307,
    KEY_F19           = 308,
    KEY_F20           = 309,
    KEY_F21           = 310,
    KEY_F22           = 311,
    KEY_F23           = 312,
    KEY_F24           = 313,
    KEY_F25           = 314,
    KEY_KP_0          = 320,
    KEY_KP_1          = 321,
    KEY_KP_2          = 322,
    KEY_KP_3          = 323,
    KEY_KP_4          = 324,
    KEY_KP_5          = 325,
    KEY_KP_6          = 326,
    KEY_KP_7          = 327,
    KEY_KP_8          = 328,
    KEY_KP_9          = 329,
    KEY_KP_DECIMAL    = 330,
    KEY_KP_DIVIDE     = 331,
    KEY_KP_MULTIPLY   = 332,
    KEY_KP_SUBTRACT   = 333,
    KEY_KP_ADD        = 334,
    KEY_KP_ENTER      = 335,
    KEY_KP_EQUAL      = 336,
    KEY_LEFT_SHIFT    = 340,
    KEY_LEFT_CONTROL  = 341,
    KEY_LEFT_ALT      = 342,
    KEY_LEFT_SUPER    = 343,
    KEY_RIGHT_SHIFT   = 344,
    KEY_RIGHT_CONTROL = 345,
    KEY_RIGHT_ALT     = 346,
    KEY_RIGHT_SUPER   = 347,
    KEY_MENU          = 348,

    MOUSE_LEFT   = 1000,
    MOUSE_RIGHT  = 1001,
    MOUSE_MIDDLE = 1002
};

struct InputResult
{
    InputButton key       = KEY_NONE;
    bool pressed          = false;
    bool repeat           = false;
    ModifierKey modifiers = MODIFIER_NONE;

    inline operator bool() const { return key != KEY_NONE; }
    inline bool andHasModifier(ModifierKey mod) const { return (key != KEY_NONE) && (modifiers & mod); }
    inline bool andDoesntHaveModifer(ModifierKey mod) const
    { return (key != KEY_NONE) && !(modifiers & mod); }
};

enum CursorType : uint8_t
{
    CURSOR_NORMAL,            // regular cursor
    CURSOR_RESIZE_HORIZONTAL, // horizontal resize arrows
    CURSOR_RESIZE_VERTICAL,   // vertical resize arrows
    CURSOR_RESIZE_TLBR,
    CURSOR_RESIZE_BLTR,
    CURSOR_TEXT,      // I-beam text cursor
    CURSOR_CROSSHAIR, // plus-shaped crosshair
    CURSOR_HAND,      // grabby hand
    CURSOR_BUSY,      // loading wheel or sand-timer cursor
    CURSOR_MAX_ENUM   // invalid cursor type used for iterating the enum
};

class Window
{
public:
    glm::vec3 clear_colour = { 0.08f, 0.08f, 0.08f };

private:
    GLFWwindow* window;
    std::array<GLFWcursor*, CURSOR_MAX_ENUM> cursors;

    glm::vec2 last_frame_size;
    glm::vec2 current_frame_size;

    glm::vec2 last_mouse_position;
    glm::vec2 mouse_delta;
    float scroll_delta      = 0.0f;
    float scroll_delta_last = 0.0f;
    std::multimap<InputButton, InputResult> mouse_events;
    std::multimap<InputButton, InputResult> key_events;
    std::queue<unsigned int> char_events;

    CursorType current_cursor = CURSOR_BUSY;
    int last_cursor_priority  = 0;

    std::map<std::string, InputResult> shortcuts;

public:
    Window();
    Window(const Window& other)         = delete;
    Window(Window&& other)              = delete;
    void operator=(const Window& other) = delete;
    void operator=(Window&& other)      = delete;
    ~Window();

    void setTitle(const std::string& title);
    glm::u32vec2 getSize() const { return current_frame_size; }
    glm::u32vec2 getLastSize() const { return last_frame_size; }

    glm::vec2 getMousePosition() const { return last_mouse_position; }
    glm::vec2 getMouseDelta() const { return mouse_delta; }
    float getScrollDelta() const { return scroll_delta_last; }
    bool isMouseDown(InputButton button) const;
    InputResult wasMousePressed(InputButton button, bool consume = true);
    InputResult wasMouseReleased(InputButton button, bool consume = true);

    bool isKeyDown(uint16_t key) const;
    InputResult wasKeyPressed(uint16_t key, bool allow_repeat = true, bool consume = true);
    InputResult wasKeyReleased(uint16_t key, bool consume = true);

    unsigned int getCharEvent();

    void poll(bool clear_events = true);
    void present() const;
    bool shouldClose() const;
    void makeCurrentContext() const;

    void setCursorType(CursorType t, int priority = 0);

    void registerShortcut(const std::string& action, ModifierKey modifiers, uint16_t key);
    bool wasShortcutTriggered(const std::string& action);
    void triggerShortcut(const std::string& action);

    void writeClipboard(const std::string& value);
    std::string readClipboard();

private:
    static void keyFunction(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void charFunction(GLFWwindow* window, unsigned int codepoint);
    static void mouseFunction(GLFWwindow* window, int button, int action, int mods);
    static void scrollFunction(GLFWwindow* window, double xoffset, double yoffset);
};

}; // namespace AriaFlow