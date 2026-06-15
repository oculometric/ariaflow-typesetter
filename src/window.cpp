#include "window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad.h>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace AriaFlow;

static std::unordered_map<GLFWwindow*, Window*> windows;

void Window::keyFunction(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    KeyEvent::Modifier modifiers;
    if (mods & GLFW_MOD_ALT) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::ALT);
    if (mods & GLFW_MOD_CAPS_LOCK) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::CAPS);
    if (mods & GLFW_MOD_CONTROL) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::CTRL);
    if (mods & GLFW_MOD_NUM_LOCK) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::NUM);
    if (mods & GLFW_MOD_SHIFT) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::SHIFT);
    if (mods & GLFW_MOD_SUPER) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::SUPER);
    windows[window]->key_events.push(
        KeyEvent{ static_cast<uint16_t>(key), action == GLFW_PRESS, modifiers });
}

void Window::charFunction(GLFWwindow* window, unsigned int codepoint)
{ windows[window]->char_events.push(codepoint); }

void Window::mouseFunction(GLFWwindow* window, int button, int action, int mods)
{
    KeyEvent::Modifier modifiers;
    if (mods & GLFW_MOD_ALT) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::ALT);
    if (mods & GLFW_MOD_CAPS_LOCK) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::CAPS);
    if (mods & GLFW_MOD_CONTROL) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::CTRL);
    if (mods & GLFW_MOD_NUM_LOCK) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::NUM);
    if (mods & GLFW_MOD_SHIFT) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::SHIFT);
    if (mods & GLFW_MOD_SUPER) modifiers = (KeyEvent::Modifier)(modifiers | KeyEvent::SUPER);
    KeyEvent::Key but;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:   but = KeyEvent::MOUSE_LEFT; break;
    case GLFW_MOUSE_BUTTON_RIGHT:  but = KeyEvent::MOUSE_RIGHT; break;
    case GLFW_MOUSE_BUTTON_MIDDLE: but = KeyEvent::MOUSE_MIDDLE; break;
    }
    windows[window]->mouse_events.push(KeyEvent{ but, action == GLFW_PRESS, modifiers });
}

Window::Window()
{
    if (windows.empty()) glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(1024, 1024, "ariaflow", nullptr, nullptr);
    makeCurrentContext();
    glfwFocusWindow(window);
    glfwShowWindow(window);
    glfwSetKeyCallback(window, keyFunction);
    glfwSetCharCallback(window, charFunction);
    glfwSetMouseButtonCallback(window, mouseFunction);
    cursors[CURSOR_NORMAL]            = nullptr;
    cursors[CURSOR_RESIZE_HORIZONTAL] = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
    cursors[CURSOR_RESIZE_VERTICAL]   = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);
    cursors[CURSOR_RESIZE_TLBR]       = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    cursors[CURSOR_RESIZE_BLTR]       = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
    cursors[CURSOR_TEXT]              = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursors[CURSOR_CROSSHAIR]         = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    cursors[CURSOR_HAND]              = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
    cursors[CURSOR_BUSY]              = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);

    glfwSwapInterval(1);
    if (windows.empty())
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("failed to initialize GLAD");
    }
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

glm::u32vec2 Window::getSize() const
{
    int x, y;
    glfwGetFramebufferSize(window, &x, &y);
    return { x, y };
}

KeyEvent Window::getKeyEvent()
{
    if (key_events.empty()) return KeyEvent();
    else
    {
        KeyEvent tmp = key_events.front();
        key_events.pop();
        return tmp;
    }
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

KeyEvent Window::getMouseEvent()
{
    if (mouse_events.empty()) return KeyEvent();
    else
    {
        KeyEvent tmp = mouse_events.front();
        mouse_events.pop();
        return tmp;
    }
}

glm::vec2 Window::getMousePosition() const { return last_mouse_position; }

glm::vec2 Window::getMouseDelta() const { return mouse_delta; }

bool Window::isMouseDown(KeyEvent::Key mouse_button) const
{
    int but;
    switch (mouse_button)
    {
    case KeyEvent::MOUSE_LEFT:   but = GLFW_MOUSE_BUTTON_LEFT; break;
    case KeyEvent::MOUSE_RIGHT:  but = GLFW_MOUSE_BUTTON_RIGHT; break;
    case KeyEvent::MOUSE_MIDDLE: but = GLFW_MOUSE_BUTTON_MIDDLE; break;
    default:                     return false;
    }
    return glfwGetMouseButton(window, but);
}

void Window::poll()
{
    glfwPollEvents();
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    glm::vec2 mouse_position = { mouse_x, mouse_y };
    mouse_delta              = mouse_position - last_mouse_position;
    last_mouse_position      = mouse_position;
}

void Window::present() const
{
    glfwSetCursor(window, cursors[current_cursor]);
    glfwSwapBuffers(window);
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window); }

void Window::makeCurrentContext() const { glfwMakeContextCurrent(window); }
