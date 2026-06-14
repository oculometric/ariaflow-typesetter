#include "user_interface.h"
#include "window.h"

#include <cstring>
#include <glad.h>
#include <stb_image.h>
#include <stdexcept>

using namespace AriaFlow;

const float line_height          = 24.0f;
const float icon_size            = 24.0f;
const float spacing              = 2.0f;
const float text_push            = -2.0f;
const glm::vec4 panel_colour     = { 0.12f, 0.12f, 0.12f, 1.0f };
const glm::vec4 panel_sec_colour = { 0.3f, 0.3f, 0.3f, 1.0f };
const glm::vec3 text_colour      = { 0.9f, 0.9f, 0.9f };
const glm::vec3 text_sec_colour  = { 0.5f, 0.5f, 0.5f };

UIMenu::UIMenu() { overall_size = { 16, spacing * 2 }; }

UIMenu::~UIMenu()
{
    for (auto& item : items)
        if (item.submenu) delete item.submenu;
    items.clear();
}

void UIMenu::addButton(const std::string& text, std::function<void(void)> callback,
    const std::string& shortcut, int icon)
{
    Item item;
    item.text         = text;
    item.is_clickable = true;
    item.icon         = icon;
    item.shortcut     = shortcut;
    item.callback     = callback;

    items.push_back(item);
}

void UIMenu::addLabel(const std::string& text, int icon)
{
    Item item;
    item.text = text;
    item.icon = icon;

    items.push_back(item);
}

void UIMenu::addDivider()
{
    Item item;
    item.is_divider = true;

    items.push_back(item);
}

UIMenu* UIMenu::addSubMenu(const std::string& text, int icon)
{
    UIMenu* menu = new UIMenu();
    Item item;
    item.is_submenu   = true;
    item.submenu      = menu;
    item.text         = text;
    item.icon         = icon;
    item.is_clickable = true;

    items.push_back(item);
    return menu;
}

void UIMenu::draw(UIRenderer* r, glm::vec2 top_left)
{
    float left     = top_left.x + (spacing * 2);
    float top      = top_left.y + spacing + text_push;
    glm::vec2 size = { 16, spacing * 2 };

    bool icons_present = false;
    float icon_left    = left - spacing;
    for (auto& item : items)
    {
        if (item.icon == -1) continue;

        icons_present = true;
        left += icon_size + spacing;
        break;
    }

    for (auto& item : items)
    {
        glm::vec2 text_size;
        if (item.icon != -1)
            r->addSimple({ icon_left, top - text_push }, 19, { icon_size, icon_size }, item.icon, { 0, 0 },
                { 1, 1 });
        if (item.is_divider) text_size = { 16, line_height };
        else
            text_size = r->addText({ left, top }, 20, {}, item.text,
                item.is_clickable ? text_colour : text_sec_colour);
        item.position = { left - (spacing + spacing), top - (spacing + text_push) };
        if (icons_present) item.position.x -= icon_size + (spacing * 1);
        item.size = { text_size.x, line_height };
        if (!item.shortcut.empty()) item.size.x += (spacing * 3) + r->calculateTextWidth(item.shortcut, {});
        if (item.is_submenu) item.size.x += (spacing * 3) + icon_size;
        size.x = glm::max(item.size.x + (spacing * 3), size.x);
        size.y += line_height;
        top += line_height;
    }
    size.x += spacing + spacing;
    if (icons_present) size.x += icon_size + (spacing * 2);
    float right = icon_left + size.x;
    for (auto& item : items)
    {
        item.size.x = size.x;
        if (!item.shortcut.empty())
            r->addText({ right - (spacing * 4), item.position.y + spacing + text_push }, 19,
                { TEXT_ALIGN_RIGHT }, item.shortcut, text_sec_colour);

        if (item.is_clicked)
            r->addNineSlice(item.position, 18, item.size + glm::vec2{ 0, spacing + 1.0f }, 2,
                panel_sec_colour, 0b1111);
        if (item.is_divider)
            r->addNineSlice(item.position + glm::vec2{ (spacing * 4), line_height / 2.0f }, 20,
                { item.size.x - (spacing * 8), 4 }, 3, panel_sec_colour, 0b0001);
        if (item.is_submenu && item.is_clicked)
            item.submenu->draw(r, item.position + glm::vec2{ item.size.x, 0 });
        if (item.is_submenu)
            r->addSimple({ right - ((spacing * 2) + icon_size), item.position.y }, 20,
                { icon_size, icon_size }, 6, { 0, 0 }, { 1, 1 });
    }
    overall_size = size;
    r->addNineSlice(top_left, 15, size, 0, panel_colour, 0b1111);
}

bool insideRect(glm::vec2 point, glm::vec2 top_left, glm::vec2 size)
{
    if (point.x < top_left.x) return false;
    if (point.y < top_left.y) return false;
    if (point.x >= top_left.x + size.x) return false;
    if (point.y >= top_left.y + size.y) return false;

    return true;
}

bool UIMenu::checkInput(Window* w, glm::vec2 top_left)
{
    glm::vec2 mouse  = w->getMousePosition();
    bool down        = w->isMouseDown(KeyEvent::MOUSE_LEFT);
    bool inside_menu = insideRect(mouse, top_left, overall_size);

    for (auto& item : items)
    {
        if (!item.is_clickable) continue;
        bool mouse_inside = insideRect(mouse, item.position, item.size);
        if (item.is_submenu)
        {
            bool mouse_inside_menu = false;
            if (item.is_clicked)
                mouse_inside_menu =
                    item.submenu->checkInput(w, item.position + glm::vec2{ item.size.x, 0 });
            item.is_clicked = mouse_inside | mouse_inside_menu;
            inside_menu |= mouse_inside_menu;
        }
        else
        {
            item.is_clicked = (mouse_inside && down);
            if (mouse_inside)
            {
                auto evt = w->getMouseEvent();
                while (evt.key != 0)
                {
                    if (evt.key == KeyEvent::MOUSE_LEFT && evt.pressed == false)
                    {
                        if (item.callback != nullptr) item.callback();
                        inside_menu = false;
                        break;
                    }
                    evt = w->getMouseEvent();
                }
            }
        }
    }

    return inside_menu;
}

UIRootMenu::UIRootMenu() {}

UIRootMenu::~UIRootMenu()
{
    for (auto& item : items)
        if (item.submenu) delete item.submenu;
    items.clear();
}

void UIRootMenu::addButton(const std::string& text, std::function<void(void)> callback, int icon)
{
    Item item;
    item.text         = text;
    item.is_clickable = true;
    item.icon         = icon;
    item.callback     = callback;

    items.push_back(item);
}

void UIRootMenu::addLabel(const std::string& text, int icon)
{
    Item item;
    item.text = text;
    item.icon = icon;

    items.push_back(item);
}

UIMenu* UIRootMenu::addSubMenu(const std::string& text, int icon)
{
    UIMenu* menu = new UIMenu();
    Item item;
    item.is_submenu   = true;
    item.submenu      = menu;
    item.text         = text;
    item.icon         = icon;
    item.is_clickable = true;

    items.push_back(item);
    return menu;
}

void UIRootMenu::draw(UIRenderer* r, float width)
{
    float bottom = line_height + (spacing * 2);
    r->addNineSlice({ 0, 0 }, 16, { width, bottom }, 0, panel_colour, 0b0010);
    float left = spacing;
    float top  = spacing;

    for (auto& item : items)
    {
        item.size = { spacing * 4, bottom };
        float tmp = left + spacing;
        if (item.icon != -1)
        {
            r->addSimple({ left, top }, 20, { icon_size, icon_size }, item.icon, { 0, 0 }, { 1, 1 });
            item.size.x += icon_size + spacing;
            tmp += icon_size + spacing;
        }
        glm::vec2 text_size = r->addText({ tmp, top + text_push }, 20, {}, item.text,
            item.is_clickable ? text_colour : text_sec_colour);
        item.position       = { left - spacing, 0 };
        if (text_size.x > 0) item.size.x += (text_size.x + (spacing * 2));
        left += item.size.x;

        if (item.is_clicked) r->addNineSlice(item.position, 18, item.size, 3, panel_sec_colour, 0b0010);
        if (item.is_submenu && item.is_clicked)
            item.submenu->draw(r, item.position + glm::vec2{ 0, item.size.y });
    }
}

void UIRootMenu::checkInput(Window* w)
{
    float bottom     = line_height + (spacing * 2);
    glm::vec2 mouse  = w->getMousePosition();
    bool down        = w->isMouseDown(KeyEvent::MOUSE_LEFT);
    bool inside_menu = insideRect(mouse, { 0, 0 }, { 100000, bottom });

    if (!is_menu_open)
    {
        if (down) is_menu_open = true;
        else
        {
            for (auto& item : items) item.is_clicked = false;
        }
        auto evt = w->getMouseEvent();
        while (evt.key != 0) evt = w->getMouseEvent();
        return;
    }

    for (auto& item : items)
    {
        if (!item.is_clickable) continue;
        bool mouse_inside = insideRect(mouse, item.position, item.size);
        if (item.is_submenu)
        {
            bool mouse_inside_menu = false;
            if (item.is_clicked)
                mouse_inside_menu =
                    item.submenu->checkInput(w, item.position + glm::vec2{ 0, item.size.y });
            item.is_clicked = mouse_inside | mouse_inside_menu;
            inside_menu |= mouse_inside_menu;
        }
        else
        {
            item.is_clicked = (mouse_inside && down);
            if (mouse_inside)
            {
                auto evt = w->getMouseEvent();
                while (evt.key != 0)
                {
                    if (evt.key == KeyEvent::MOUSE_LEFT && evt.pressed == false)
                    {
                        if (item.callback != nullptr) item.callback();
                        is_menu_open = false;
                        release_flag = false;
                        return;
                    }
                    evt = w->getMouseEvent();
                }
            }
        }
    }

    bool was_released = false;
    auto evt          = w->getMouseEvent();
    while (evt.key != 0)
    {
        if (evt.key == KeyEvent::MOUSE_LEFT && evt.pressed == false)
        {
            was_released = true;
            break;
        }
        evt = w->getMouseEvent();
    }

    if (!inside_menu)
    {
        is_menu_open = false;
        release_flag = false;
    }
    else if (inside_menu && was_released)
    {
        if (release_flag)
        {
            is_menu_open = false;
            release_flag = false;
        }
        else
            release_flag = true;
    }
}

UIButton::UIButton(const std::string& text, std::function<void(void)> callback, int icon)
{
    message       = text;
    icon_index    = icon;
    callback_func = callback;
}

glm::vec2 UIButton::getSize(UIRenderer* r)
{
    float width = 0;
    if (icon_index != -1) width += icon_size + (spacing * 2);
    if (!message.empty()) width += r->calculateTextWidth(message, {}) + (spacing * 2);
    if (width == 0) width = 16;

    float height = 16;
    if (icon_index != -1) height = glm::max(height, icon_size + (spacing * 2));
    if (!message.empty()) height = glm::max(height, line_height + (spacing * 2));

    last_size = glm::vec2{ width + spacing, height };

    return last_size;
}

void UIButton::draw(UIRenderer* r, glm::vec2 position)
{
    r->addNineSlice(position, 0, getSize(r), is_pressed ? 2 : 0, panel_colour, 0b1111);
    glm::vec2 pos = position + glm::vec2{ spacing, spacing };
    if (icon_index != -1)
    {
        r->addSimple(pos, 1, { icon_size, icon_size }, icon_index, { 0, 0 }, { 1, 1 });
        pos.x += icon_size + (spacing * 2);
    }
    if (!message.empty()) r->addText(pos + glm::vec2{ 0, text_push }, 1, {}, message, text_colour);
}

void UIButton::checkInput(Window* w, glm::vec2 position)
{
    bool inside = insideRect(w->getMousePosition(), position, last_size);
    is_pressed  = inside && w->isMouseDown(KeyEvent::MOUSE_LEFT);
    if (inside)
    {
        auto evt = w->getMouseEvent();
        while (evt.key != 0)
        {
            if (evt.key == KeyEvent::MOUSE_LEFT && evt.pressed == false)
            {
                if (callback_func != nullptr) callback_func();
                break;
            }
            evt = w->getMouseEvent();
        }
    }
}

UILabel::UILabel(const std::string& text, TextAlign align, TextFlags flags, int icon)
{
    message    = text;
    icon_index = icon;
    direction  = align;
    settings   = flags;
}

void UILabel::draw(UIRenderer* r, glm::vec2 position)
{
    glm::vec2 pos = position;
    if (direction == TEXT_ALIGN_RIGHT) pos -= icon_size;
    if (icon_index != -1)
    {
        r->addSimple(pos, 1, { icon_size, icon_size }, icon_index, { 0, 0 }, { 1, 1 });
        if (direction == TEXT_ALIGN_RIGHT)
            pos.x -= (spacing * 2);
        else
            pos.x += icon_size + (spacing * 2);
    }
    if (!message.empty())
        r->addText(pos + glm::vec2{ 0, text_push }, 1, { direction, settings }, message, text_colour);
}

UIPanel::UIPanel(glm::vec4 fill, int layer, uint8_t borders)
{
    fill_colour = fill;
    layer_index = layer;
    border_flags = borders;
}

void AriaFlow::UIPanel::draw(UIRenderer* r, glm::vec2 position, glm::vec2 size)
{
    r->addNineSlice(position, 0, size, layer_index, fill_colour, border_flags);
}
