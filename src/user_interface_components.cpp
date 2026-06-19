#include "user_interface.h"
#include "window.h"

using namespace AriaFlow;

UIButton::UIButton(const std::string& text, std::function<void(void)> callback, int icon, glm::vec2 offset)
{
    message       = text;
    icon_index    = icon;
    callback_func = callback;
    position      = offset;
}

glm::vec2 UIButton::getSize(std::shared_ptr<UIRenderer> r)
{
    float width = 0;
    if (icon_index != -1) width += icon_size + (spacing * 2);
    if (!message.empty()) width += r->calculateTextWidth(message, {}) + (spacing * 3);
    if (width == 0) width = 16;

    float height = 16;
    if (icon_index != -1) height = glm::max(height, icon_size + (spacing * 2));
    if (!message.empty()) height = glm::max(height, line_height + (spacing * 2));

    size = glm::vec2{ width, height };

    return size;
}

void UIButton::draw(std::shared_ptr<UIRenderer> r)
{
    r->addNineSlice(position, z, getSize(r), (is_pressed && mouse_inside) ? 2 : 0, panel_colour, 0b1111);
    glm::vec2 pos = position + glm::vec2{ spacing, spacing };
    if (icon_index != -1)
    {
        r->addSimple(pos, z, { icon_size, icon_size }, icon_index, { 0, 0 }, { 1, 1 });
        pos.x += icon_size + (spacing * 2);
    }
    if (!message.empty()) r->addText(pos + glm::vec2{ 0, text_push }, z, {}, message, text_colour);
}

void UIButton::checkInput(std::shared_ptr<Window> w)
{
    mouse_inside = insideRect(w->getMousePosition(), position, size);
    if (!is_pressed && mouse_inside && checkForMouseDown(w)) is_pressed = true;
    if (is_pressed && !w->isMouseDown(KeyEvent::MOUSE_LEFT))
    {
        if (mouse_inside)
        {
            if (callback_func != nullptr) callback_func();
        }
        is_pressed = false;
    }
}

UILabel::UILabel(const std::string& text, TextAlign align, TextFlags flags, int icon, glm::vec2 offset)
{
    message    = text;
    icon_index = icon;
    direction  = align;
    settings   = flags;
    position   = offset;
}

void UILabel::draw(std::shared_ptr<UIRenderer> r)
{
    glm::vec2 pos = position;
    glm::vec2 sz  = size;
    if (icon_index != -1)
    {
        sz.x -= icon_size + (spacing * 2);
        if (direction == TEXT_ALIGN_RIGHT) pos.x -= icon_size + (spacing * 2);
        else
            pos.x += icon_size + (spacing * 2);
        r->addSimple((direction == TEXT_ALIGN_RIGHT ? (position - glm::vec2{ (icon_size + spacing), 0 })
                                                    : position) +
                         spacing,
            z, { icon_size, icon_size }, icon_index, { 0, 0 }, { 1, 1 });
    }
    if (!message.empty())
        r->addText(pos + glm::vec2{ direction == TEXT_ALIGN_RIGHT
                                        ? sz.x
                                        : (direction == TEXT_ALIGN_CENTER ? sz.x / 2.0f : 0.0f),
                             text_push },
            z, { direction, settings, true, wrap, false, sz, 0, text_size }, message, text_colour);
}

UIPanel::UIPanel(glm::vec4 fill, int layer, uint8_t borders, glm::vec2 offset, glm::vec2 dimensions) :
    UIElement(offset, dimensions)
{
    fill_colour  = fill;
    layer_index  = layer;
    border_flags = borders;
}

void UIPanel::draw(std::shared_ptr<UIRenderer> r)
{ r->addNineSlice(position, z, size, layer_index, fill_colour, border_flags); }

UIGrabbable::UIGrabbable(CursorType cursor_indicator) : UIElement({ 0, 0 }, { 0, 0 })
{
    grabbed = false;
    cursor  = cursor_indicator;
}

glm::vec2 UIGrabbable::checkInput(std::shared_ptr<Window> w, glm::vec2 position, glm::vec2 area_size)
{
    bool inside = insideRect(w->getMousePosition(), position, area_size);
    if (inside && !grabbed)
    {
        if (checkForMouseDown(w)) grabbed = true;
    }

    if (grabbed)
    {
        position += w->getMouseDelta();

        if (!w->isMouseDown(KeyEvent::MOUSE_LEFT)) grabbed = false;
    }

    if (inside || grabbed) w->setCursorType(cursor, 2);

    return position;
}
