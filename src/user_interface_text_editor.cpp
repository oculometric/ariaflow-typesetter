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
        custom_r->addText(p - glm::vec2{ (spacing * 2), 0 }, z, format2, std::to_string(line_index),
            text_sec_colour);
        custom_r->addText(p, z, format, data_source->getData().substr(line.first, line.second - line.first),
            text_colour);
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
            if (evt == 'P')
                scroll += 8.0f;
            else if (evt == 'O')
                scroll -= 8.0f;
            evt = w->getCharEvent();
        }
    }
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
