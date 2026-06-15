#include "document.h"
#include "user_interface.h"

using namespace AriaFlow;

const TextFormatting format{ TEXT_ALIGN_LEFT, TEXT_FLAGS_NONE, true, false, false, {}, 1, 23 };

void UITextEditor::draw(UIRenderer* _r)
{
    TextFormatting format2 = format;
    format2.align          = TEXT_ALIGN_RIGHT;
    format2.size           = 12.5f;
    glm::vec2 p            = position + glm::vec2{ left_margin, (spacing * 2) - scroll };
    size_t line_index      = 1;
    for (const auto& line : lines)
    {
        if (p.y >= position.y + size.y) break;
        if (p.y <= position.y)
        {
            p.y += line_height;
            ++line_index;
            continue;
        }
        TextFormatting format3 = format2;
        format3.clip           = true;
        format3.clip_bounds    = { 100000.0f, (position.y + size.y) - p.y };
        custom_r->addText(p - glm::vec2{ (spacing * 2), 0 }, z, format3, std::to_string(line_index),
            text_sec_colour);
        format3             = format;
        format3.clip        = true;
        format3.clip_bounds = { 100000.0f, (position.y + size.y) - p.y };
        custom_r->addText(p, z, format3,
            data_source->getData().substr(line.first, line.second - line.first), text_colour);
        p.y += line_height;
        ++line_index;
    }
}

void UITextEditor::checkInput(Window* w)
{
    if (size != last_checked_size && custom_r)
    {
        last_checked_size = size;
        updateLines();
    }

    glm::vec2 mouse = w->getMousePosition();
    if (insideRect(mouse, position, size))
    {
        auto evt = w->getCharEvent();
        while (evt != 0)
        {
            if (evt == 'P') scroll += 8.0f;
            else if (evt == 'O')
                scroll -= 8.0f;
            evt = w->getCharEvent();
        }
    }
    float lines_tall = size.y / line_height;
    scroll           = glm::clamp(scroll, 0.0f,
        line_height * glm::max(static_cast<float>(lines.size()) - glm::max(lines_tall - 8.0f, 1.0f), 0.0f));
    // TODO: text editor input handling (big)
    // TODO: keyboard navigation, mouse navigation, key inputs, selection
}

void UITextEditor::updateLines()
{
    if (!data_source) lines.clear();
    else
    {
        float single_char_width = custom_r->calculateTextWidth("a", format);
        float multi_char_width  = custom_r->calculateTextWidth("aa", format) - single_char_width;
        float line_width        = getContentWidth();
        size_t chars_per_line   = 1;
        line_width -= single_char_width;
        while (line_width > multi_char_width)
        {
            line_width -= multi_char_width;
            ++chars_per_line;
        }
        lines = data_source->splitToLines(chars_per_line);
    }
    updateCursor();
}

void UITextEditor::updateCursor()
{
    // TODO: clamp cursor index
    // TODO: clamp cursor line
}

float UITextEditor::getContentWidth() { return size.x - (left_margin + right_margin); }
