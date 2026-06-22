#include "window.h"

#define GLFW_INCLUDE_NONE
#include "icon.h"

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
