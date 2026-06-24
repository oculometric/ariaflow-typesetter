/*
 * BBUI interface rendering toolkit.
 * Copyright (C) 2026  cassette costen

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if __cplusplus < 202002L
#error BBUI requires C++20 to be compiled
#endif

#pragma region BBUI_DECLARATIONS


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

#pragma endregion BBUI_DECLARATIONS

#pragma region BBUI_IMPLEMENTATIONS
#if defined(BBUI_IMPLEMENTATION)


#include <GLFW/glfw3.h>
#include <glad.h>
#include <iostream>
#include <stb_image.h>
#include <stdexcept>
#include <unordered_map>

using namespace AriaFlow;

static std::unordered_map<GLFWwindow*, Window*> windows;

void Window::keyFunction(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ModifierKey modifiers = MODIFIER_NONE;
    if (mods & GLFW_MOD_ALT) modifiers = modifiers | MODIFIER_ALT;
    if (mods & GLFW_MOD_CAPS_LOCK) modifiers = modifiers | MODIFIER_CAPS;
    if (mods & GLFW_MOD_CONTROL) modifiers = modifiers | MODIFIER_CTRL;
    if (mods & GLFW_MOD_NUM_LOCK) modifiers = modifiers | MODIFIER_NUM;
    if (mods & GLFW_MOD_SHIFT) modifiers = modifiers | MODIFIER_SHIFT;
    if (mods & GLFW_MOD_SUPER) modifiers = modifiers | MODIFIER_SUPER;
    InputResult result = { static_cast<InputButton>(key), action == GLFW_PRESS, action == GLFW_REPEAT,
        modifiers };
    windows[window]->key_events.insert({ result.key, result });

    if (action == GLFW_PRESS)
    {
        for (auto& pair : windows[window]->shortcuts)
        {
            if (pair.second.key == static_cast<uint16_t>(key) &&
                pair.second.modifiers ==
                    (modifiers & (MODIFIER_ALT | MODIFIER_CTRL | MODIFIER_SHIFT | MODIFIER_SUPER)))
                pair.second.pressed = true;
        }
    }
}

void Window::charFunction(GLFWwindow* window, unsigned int codepoint)
{ windows[window]->char_events.push(codepoint); }

void Window::mouseFunction(GLFWwindow* window, int button, int action, int mods)
{
    ModifierKey modifiers = MODIFIER_NONE;
    if (mods & GLFW_MOD_ALT) modifiers = modifiers | MODIFIER_ALT;
    if (mods & GLFW_MOD_CAPS_LOCK) modifiers = modifiers | MODIFIER_CAPS;
    if (mods & GLFW_MOD_CONTROL) modifiers = modifiers | MODIFIER_CTRL;
    if (mods & GLFW_MOD_NUM_LOCK) modifiers = modifiers | MODIFIER_NUM;
    if (mods & GLFW_MOD_SHIFT) modifiers = modifiers | MODIFIER_SHIFT;
    if (mods & GLFW_MOD_SUPER) modifiers = modifiers | MODIFIER_SUPER;
    InputButton but;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:   but = MOUSE_LEFT; break;
    case GLFW_MOUSE_BUTTON_RIGHT:  but = MOUSE_RIGHT; break;
    case GLFW_MOUSE_BUTTON_MIDDLE: but = MOUSE_MIDDLE; break;
    }

    InputResult result = { static_cast<InputButton>(but), action == GLFW_PRESS, action == GLFW_REPEAT,
        modifiers };
    windows[window]->mouse_events.insert({ result.key, result });
}

void Window::scrollFunction(GLFWwindow* window, double xoffset, double yoffset)
{ windows[window]->scroll_delta += static_cast<float>(yoffset); }

Window::Window()
{
    // init glfw if it isnt already
    if (windows.empty())
    {
#if defined(_WIN32)
#else
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
        glfwInit();
    }
    // create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(1024, 1024, "ariaflow", nullptr, nullptr);
    glfwFocusWindow(window);
    glfwShowWindow(window);
    glfwSwapInterval(1);

    // get size
    int x, y;
    glfwGetFramebufferSize(window, &x, &y);
    last_frame_size = current_frame_size = { x, y };

    // configure window input
    glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    glfwSetKeyCallback(window, keyFunction);
    glfwSetCharCallback(window, charFunction);
    glfwSetMouseButtonCallback(window, mouseFunction);
    glfwSetScrollCallback(window, scrollFunction);

    // load cursors
    cursors[CURSOR_NORMAL]            = nullptr;
    cursors[CURSOR_RESIZE_HORIZONTAL] = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
    cursors[CURSOR_RESIZE_VERTICAL]   = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);
    cursors[CURSOR_RESIZE_TLBR]       = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    cursors[CURSOR_RESIZE_BLTR]       = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
    cursors[CURSOR_TEXT]              = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursors[CURSOR_CROSSHAIR]         = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    cursors[CURSOR_HAND]              = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
    cursors[CURSOR_BUSY]              = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);

    // set icon
    GLFWimage image;
    int img_channels;
    image.pixels = stbi_load_from_memory(icon, static_cast<int>(icon_size), &image.width, &image.height,
        &img_channels, STBI_rgb_alpha);
    glfwSetWindowIcon(window, 1, &image);
    stbi_image_free(image.pixels);

    // configure opengl
    makeCurrentContext();
    if (windows.empty())
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("failed to initialize GLAD");
    }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthRange(-1000.0f, 1000.0f);
    glDisable(GL_CULL_FACE);

    // insert into the window map for input redirection
    windows[window] = this;
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwWaitEvents();
    windows.erase(window);
    if (windows.empty()) glfwTerminate();
}

void Window::setTitle(const std::string& title) { glfwSetWindowTitle(window, title.c_str()); }

bool Window::isMouseDown(InputButton button) const
{
    int but;
    switch (button)
    {
    case MOUSE_LEFT:   but = GLFW_MOUSE_BUTTON_LEFT; break;
    case MOUSE_RIGHT:  but = GLFW_MOUSE_BUTTON_RIGHT; break;
    case MOUSE_MIDDLE: but = GLFW_MOUSE_BUTTON_MIDDLE; break;
    default:           return false;
    }
    return glfwGetMouseButton(window, but);
}

InputResult Window::wasMousePressed(InputButton button, bool consume)
{
    auto [first, last] = mouse_events.equal_range(button);
    for (auto it = first; it != last; ++it)
    {
        if (it->second.pressed)
        {
            InputResult result = it->second;
            if (consume) mouse_events.erase(it);
            return result;
        }
    }
    return InputResult{};
}

InputResult Window::wasMouseReleased(InputButton button, bool consume)
{
    auto [first, last] = mouse_events.equal_range(button);
    for (auto it = first; it != last; ++it)
    {
        if (!it->second.pressed)
        {
            InputResult result = it->second;
            if (consume) mouse_events.erase(it);
            return result;
        }
    }
    return InputResult{};
}

bool Window::isKeyDown(uint16_t key) const { return glfwGetKey(window, key) == GLFW_PRESS; }

InputResult Window::wasKeyPressed(uint16_t key, bool allow_repeat, bool consume)
{
    auto [first, last] = key_events.equal_range(static_cast<InputButton>(key));
    for (auto it = first; it != last; ++it)
    {
        if (it->second.pressed || (allow_repeat && it->second.repeat))
        {
            InputResult result = it->second;
            if (consume) key_events.erase(it);
            return result;
        }
    }
    return InputResult{};
}

InputResult Window::wasKeyReleased(uint16_t key, bool consume)
{
    auto [first, last] = key_events.equal_range(static_cast<InputButton>(key));
    for (auto it = first; it != last; ++it)
    {
        if (!it->second.pressed)
        {
            InputResult result = it->second;
            if (consume) key_events.erase(it);
            return result;
        }
    }
    return InputResult{};
}

unsigned int Window::getCharEvent()
{
    if (char_events.empty()) return 0;
    else
    {
        unsigned int tmp = char_events.front();
        char_events.pop();
        return tmp;
    }
}

void Window::poll(bool clear_events)
{
    // reset things
    last_cursor_priority = 0;
    if (clear_events)
    {
        while (!char_events.empty()) char_events.pop();
        key_events.clear();
        mouse_events.clear();
    }

    glfwPollEvents();
    last_frame_size = current_frame_size;
    int x, y;
    glfwGetFramebufferSize(window, &x, &y);
    if (x > 2 && y > 2) current_frame_size = { x, y };
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    glm::vec2 mouse_position = { mouse_x, mouse_y };
    mouse_delta              = mouse_position - last_mouse_position;
    last_mouse_position      = mouse_position;
    scroll_delta_last        = scroll_delta;
    scroll_delta             = 0.0f;
}

void Window::present() const
{
    glfwSetCursor(window, cursors[current_cursor]);
    glfwSwapBuffers(window);
    glViewport(0, 0, getSize().x, getSize().y);
    glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, 1.0f);
    glClearDepth(-1000.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Window::shouldClose() const
{
    bool tmp = glfwWindowShouldClose(window);
    glfwSetWindowShouldClose(window, false);
    return tmp;
}

void Window::makeCurrentContext() const { glfwMakeContextCurrent(window); }

void Window::setCursorType(CursorType t, int priority)
{
    if (priority >= last_cursor_priority)
    {
        current_cursor       = t;
        last_cursor_priority = priority;
    }
}

void Window::registerShortcut(const std::string& action, ModifierKey modifiers, uint16_t key)
{ shortcuts[action] = InputResult{ static_cast<InputButton>(key), false, false, modifiers }; }

bool Window::wasShortcutTriggered(const std::string& action)
{
    if (!shortcuts.count(action)) return false;
    if (shortcuts[action].pressed)
    {
        shortcuts[action].pressed = false;
        return true;
    }
    return false;
}

void Window::triggerShortcut(const std::string& action)
{
    if (shortcuts.count(action)) shortcuts[action].pressed = true;
}

void Window::writeClipboard(const std::string& value) { glfwSetClipboardString(window, value.c_str()); }

std::string Window::readClipboard() { return glfwGetClipboardString(window); }


#endif
#pragma endregion BBUI_IMPLEMENTATIONS
