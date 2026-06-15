#include "document.h"
#include "user_interface.h"

using namespace AriaFlow;

const TextFormatting format{ TEXT_ALIGN_LEFT, TEXT_FLAGS_NONE, true, false, false, {}, 1, 23 };

void UITextEditor::draw(UIRenderer* _r)
{
    glm::vec2 p = position;
    for (const auto& line : lines)
    {
        custom_r->addText(p, z, format, data_source->getData().substr(line.first, line.second - line.first),
            text_colour);
        p.y += line_height;
    }
}

void UITextEditor::checkInput(Window* w)
{
    if (size != last_checked_size && custom_r)
    {
        last_checked_size = size;
        updateLines();
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
        float line_width        = size.x;
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
