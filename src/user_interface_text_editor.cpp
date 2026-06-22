#include "document.h"
#include "user_interface.h"

using namespace AriaFlow;

const TextFormatting format{ TEXT_ALIGN_LEFT, TEXT_FLAGS_NONE, true, false, false, {}, 1, 23 };

void UITextEditor::draw(std::shared_ptr<UIRenderer> _r)
{
    TextFormatting format2     = format;
    format2.align              = TEXT_ALIGN_RIGHT;
    format2.size               = 12.5f;
    const glm::vec2 text_start = position + glm::vec2{ left_margin, (spacing * 2) - scroll };

    glm::vec2 p       = text_start;
    size_t line_index = 0;
    bool line_updated = true;

    for (const auto& line : lines)
    {
        line_updated = true;
        if (line_index == 0) ++line_index;
        else
        {
            if (data_source->getData()[line.start - 1] == '\n') ++line_index;
            else
                line_updated = false;
        }

        if (p.y >= position.y + size.y) break;
        if (p.y <= position.y)
        {
            p.y += line_height;
            continue;
        }
        // line number
        if (line_updated)
        {
            TextFormatting format3 = format2;
            format3.clip           = true;
            format3.clip_bounds    = { 100000.0f, (position.y + size.y) - p.y };
            custom_r->addText(p - glm::vec2{ (spacing * 2), 0 }, z, format3, std::to_string(line_index),
                text_sec_colour);
        }
        // text content
        {
            TextFormatting format3            = format;
            format3.clip                      = true;
            format3.clip_bounds               = { 100000.0f, (position.y + size.y) - p.y };
            static const glm::vec3 colours[4] = {
                glm::vec3{ 1, 0, 0 },
                glm::vec3{ 0, 1, 0 },
                glm::vec3{ 0, 0, 1 },
                glm::vec3{ 1, 1, 1 }
            };
            std::vector<std::pair<size_t, glm::vec3>> cols;
            for (const auto& p : line.colours) cols.emplace_back(p.first, colours[p.second]);
            custom_r->addText(p, z, format3,
                data_source->getData().substr(line.start, line.end - line.start), cols);
        }
        // selection highlight
        {
            size_t min = glm::min(cursor_index, selection_other_end_index);
            size_t max = glm::max(cursor_index, selection_other_end_index);

            if (max >= line.start && min <= line.end)
            {
                size_t hmin  = glm::max(min, line.start);
                size_t hmax  = glm::min(max, line.end);
                float height = line_height;
                if (max > line.end) ++hmax;
                if (hmin != hmax)
                {
                    float char_width = custom_r->calculateTextWidth("a", format);
                    custom_r->addNineSlice(p + glm::vec2{ findIndexOffset(hmin, line.start) - 1, -spacing },
                        z + 0.1f,
                        glm::vec2{ (findIndexOffset(hmax, line.start) - findIndexOffset(hmin, line.start)) +
                                       2,
                            height },
                        0, glm::vec4{ 0.8f, 0.8f, 0.8f, 0.2f }, 0b0000);
                }
            }
        }

        p.y += line_height;
    }

    // cursor
    {
        glm::vec2 cursor_offset       = findCursorOffset(cursor_column, cursor_line);
        glm::vec2 cursor_top_left     = text_start + cursor_offset - glm::vec2{ 1, (spacing * 2) };
        glm::vec2 cursor_bottom_right = cursor_top_left + glm::vec2{ 2, line_height + (spacing * 2.0f) };
        if (cursor_bottom_right.y > position.y && cursor_top_left.y < position.y + size.y)
        {
            cursor_top_left.y     = glm::max(cursor_top_left.y, position.y);
            cursor_bottom_right.y = glm::min(cursor_bottom_right.y, position.y + size.y);
            custom_r->addNineSlice(cursor_top_left, z + 0.1f, cursor_bottom_right - cursor_top_left, 0,
                glm::vec4{ text_colour, 1 }, 0b0000);
        }
    }
}

void UITextEditor::checkInput(std::shared_ptr<Window> w)
{
    if (size != last_checked_size && custom_r)
    {
        last_checked_size = size;
        updateLines();
        auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
        cursor_column = a;
        cursor_line   = b;
        updateCursorIndex(true);
    }

    glm::vec2 mouse = w->getMousePosition();
    if (insideRect(mouse, position, size))
    {
        auto evt = w->getCharEvent();
        while (evt != 0)
        {
            data_source->pushHistory();
            eraseSelection();
            data_source->getData().insert(data_source->getData().begin() + cursor_index, (char)evt);
            ++cursor_index;
            updateLines();
            auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
            cursor_column = a;
            cursor_line   = b;
            updateCursorIndex(false);
            scrollCursorOnscreen();
            evt = w->getCharEvent();
        }

        auto evt2 = w->getKeyEvent();
        while (evt2.key != 0)
        {
            if (!(evt2.pressed || evt2.repeat))
            {
                evt2 = w->getKeyEvent();
                continue;
            }
            if (evt2.key == KeyEvent::KEY_DOWN ||
                (evt2.key == KeyEvent::KEY_KP_2 && !(evt2.modifiers & KeyEvent::NUM)))
            {
                if (evt2.modifiers & KeyEvent::CTRL)
                {
                    if (cursor_index == data_source->getData().size()) {}
                    else
                    {
                        ++cursor_index;
                        while (cursor_index < data_source->getData().size() &&
                               data_source->getData()[cursor_index - 1] != '\n')
                            ++cursor_index;
                    }
                    auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                    cursor_column = a;
                    cursor_line   = b;
                    updateCursorIndex(false);
                    scrollCursorOnscreen();
                }
                else
                {
                    if (cursor_line == lines.size() - 1 && !lines.empty())
                    {
                        cursor_column = lines[cursor_line].end;
                    }
                    cursor_line = glm::min(cursor_line + 1, lines.size() - 1);
                    updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                    scrollCursorOnscreen();
                    if (evt2.modifiers & KeyEvent::ALT) scroll += line_height;
                }
            }
            if (evt2.key == KeyEvent::KEY_UP ||
                (evt2.key == KeyEvent::KEY_KP_8 && !(evt2.modifiers & KeyEvent::NUM)))
            {
                if (evt2.modifiers & KeyEvent::CTRL)
                {
                    if (cursor_index == data_source->getData().size()) {}
                    else
                    {
                        --cursor_index;
                        while (cursor_index > 0 && data_source->getData()[cursor_index - 1] != '\n')
                            --cursor_index;
                    }
                    auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                    cursor_column = a;
                    cursor_line   = b;
                    updateCursorIndex(false);
                    scrollCursorOnscreen();
                }
                else
                {
                    if (cursor_line > 0) --cursor_line;
                    updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                    scrollCursorOnscreen();
                    if (evt2.modifiers & KeyEvent::ALT) scroll -= line_height;
                }
            }
            if (evt2.key == KeyEvent::KEY_RIGHT ||
                (evt2.key == KeyEvent::KEY_KP_6 && !(evt2.modifiers & KeyEvent::NUM)))
            {
                if (evt2.modifiers & KeyEvent::CTRL)
                {
                    cursor_index  = data_source->findNextWord(cursor_index);
                    auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                    cursor_column = a;
                    cursor_line   = b;
                    updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                    scrollCursorOnscreen();
                }
                else
                {
                    size_t line_length = 0;
                    if (cursor_line < lines.size())
                        line_length = lines[cursor_line].end - lines[cursor_line].start;
                    size_t effective_cursor_column = glm::min(line_length, cursor_column);
                    if (effective_cursor_column == line_length)
                    {
                        if (cursor_line < lines.size() - 1)
                        {
                            ++cursor_line;
                            cursor_column = 0;
                        }
                        else
                            cursor_column = effective_cursor_column;
                    }
                    else
                        ++cursor_column;
                    updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                    scrollCursorOnscreen();
                }
            }
            if (evt2.key == KeyEvent::KEY_LEFT ||
                (evt2.key == KeyEvent::KEY_KP_4 && !(evt2.modifiers & KeyEvent::NUM)))
            {
                if (evt2.modifiers & KeyEvent::CTRL)
                {
                    cursor_index  = data_source->findPrevWord(cursor_index);
                    auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                    cursor_column = a;
                    cursor_line   = b;
                    updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                    scrollCursorOnscreen();
                }
                else
                {
                    size_t line_length = 0;
                    if (cursor_line < lines.size())
                        line_length = lines[cursor_line].end - lines[cursor_line].start;
                    size_t effective_cursor_column = glm::min(line_length, cursor_column);
                    if (effective_cursor_column == 0)
                    {
                        if (cursor_line > 0)
                        {
                            --cursor_line;
                            cursor_column = lines[cursor_line].end - lines[cursor_line].start;
                        }
                        else
                            cursor_column = effective_cursor_column;
                    }
                    else
                        cursor_column = effective_cursor_column - 1;
                    updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                    scrollCursorOnscreen();
                }
            }
            if (evt2.key == KeyEvent::KEY_HOME ||
                (evt2.key == KeyEvent::KEY_KP_7 && !(evt2.modifiers & KeyEvent::NUM)))
            {
                cursor_column = 0;
                cursor_line   = 0;
                updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                scrollCursorOnscreen();
            }
            if (evt2.key == KeyEvent::KEY_END ||
                (evt2.key == KeyEvent::KEY_KP_1 && !(evt2.modifiers & KeyEvent::NUM)))
            {
                if (lines.empty())
                {
                    cursor_line   = 0;
                    cursor_column = 0;
                }
                else
                {
                    cursor_line   = lines.size() - 1;
                    cursor_column = lines[cursor_line].end;
                }
                updateCursorIndex(evt2.modifiers & KeyEvent::SHIFT);
                scrollCursorOnscreen();
            }
            if (evt2.key == KeyEvent::KEY_ESCAPE)
            {
                updateCursorIndex(false);
                scrollCursorOnscreen();
            }
            if (evt2.key == KeyEvent::KEY_BACKSPACE)
            {
                if (cursor_index != selection_other_end_index)
                {
                    data_source->pushHistory();
                    eraseSelection();
                }
                else if (cursor_index > 0)
                {
                    data_source->pushHistory();
                    data_source->getData().erase(data_source->getData().begin() + cursor_index - 1);
                    --cursor_index;
                    updateLines();
                    auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                    cursor_column = a;
                    cursor_line   = b;
                    updateCursorIndex(false);
                    scrollCursorOnscreen();
                }
            }
            if (evt2.key == KeyEvent::KEY_DELETE)
            {
                if (cursor_index != selection_other_end_index)
                {
                    data_source->pushHistory();
                    eraseSelection();
                }
                else if (cursor_index < data_source->getData().size())
                {
                    data_source->pushHistory();
                    data_source->getData().erase(data_source->getData().begin() + cursor_index);
                    updateLines();
                    auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                    cursor_column = a;
                    cursor_line   = b;
                    updateCursorIndex(false);
                    scrollCursorOnscreen();
                }
            }
            if (evt2.key == KeyEvent::KEY_ENTER)
            {
                data_source->pushHistory();
                eraseSelection();
                data_source->getData().insert(data_source->getData().begin() + cursor_index, '\n');
                ++cursor_index;
                updateLines();
                auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                cursor_column = a;
                cursor_line   = b;
                updateCursorIndex(false);
                scrollCursorOnscreen();
            }
            if (evt2.key == KeyEvent::KEY_TAB)
            {
                data_source->pushHistory();
                eraseSelection();
                size_t diff   = 4 - (cursor_column % 4);
                std::string s = std::string(diff, ' ');
                data_source->getData().insert(cursor_index, s);
                cursor_index += diff;
                updateLines();
                auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
                cursor_column = a;
                cursor_line   = b;
                updateCursorIndex(false);
                scrollCursorOnscreen();
            }
            evt2 = w->getKeyEvent();
        }

        if (!mouse_down_event_received && checkForMouseDown(w))
        {
            mouse_down_event_received = true;
            auto [a, b]               = findCursorPlacement(mouse);
            cursor_column             = a;
            cursor_line               = b;
            updateCursorIndex(w->isKeyDown(KeyEvent::KEY_LEFT_SHIFT));
        }

        if (w->isMouseDown(KeyEvent::MOUSE_LEFT) && mouse_down_event_received)
        {
            auto [a, b]   = findCursorPlacement(mouse);
            cursor_column = a;
            cursor_line   = b;
            updateCursorIndex(true);
        }
        else
            mouse_down_event_received = false;

        w->setCursorType(CursorType::CURSOR_TEXT, 1);

        scroll -= w->getScrollDelta() * line_height;
    }
    if (w->wasShortcutTriggered("select_all"))
    {
        cursor_index              = data_source->getData().size();
        auto [a, b]               = calculateColumnLineFromIndex(cursor_index);
        cursor_column             = a;
        cursor_line               = b;
        selection_other_end_index = 0;
    }
    if (w->wasShortcutTriggered("select_paragraph"))
    {
        if (cursor_index < data_source->getData().size())
        {
            auto [start, end]         = collectParagraph(cursor_index);
            selection_other_end_index = start;
            cursor_index              = end;
            auto [a, b]               = calculateColumnLineFromIndex(cursor_index);
            cursor_column             = a;
            cursor_line               = b;
            updateCursorIndex(true);
        }
    }
    if (w->wasShortcutTriggered("copy"))
    {
        if (selection_other_end_index == cursor_index)
        {
            auto [start, end] = collectParagraph(cursor_index);
            copy_buffer       = data_source->getData().substr(start, end - start) + '\n';
            w->writeClipboard(copy_buffer);
        }
        else
        {
            size_t min  = glm::min(cursor_index, selection_other_end_index);
            size_t max  = glm::max(cursor_index, selection_other_end_index);
            copy_buffer = data_source->getData().substr(min, max - min);
            w->writeClipboard(copy_buffer);
        }
    }
    if (w->wasShortcutTriggered("cut"))
    {
        if (selection_other_end_index == cursor_index)
        {
            auto [start, end] = collectParagraph(cursor_index);
            copy_buffer       = data_source->getData().substr(start, end - start) + '\n';
            w->writeClipboard(copy_buffer);
            cursor_index              = start;
            selection_other_end_index = end + 1;
            data_source->pushHistory();
            eraseSelection();
            scrollCursorOnscreen();
        }
        else
        {
            size_t min  = glm::min(cursor_index, selection_other_end_index);
            size_t max  = glm::max(cursor_index, selection_other_end_index);
            copy_buffer = data_source->getData().substr(min, max - min);
            w->writeClipboard(copy_buffer);
            data_source->pushHistory();
            eraseSelection();
            scrollCursorOnscreen();
        }
    }
    if (w->wasShortcutTriggered("paste"))
    {
        data_source->pushHistory();
        eraseSelection();
        copy_buffer = w->readClipboard();
        data_source->getData().insert(cursor_index, copy_buffer);
        cursor_index += copy_buffer.size();
        updateLines();
        auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
        cursor_column = a;
        cursor_line   = b;
        updateCursorIndex(false);
        scrollCursorOnscreen();
    }
    if (w->wasShortcutTriggered("undo"))
    {
        data_source->stepBackHistory();
        updateLines();
        auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
        cursor_column = a;
        cursor_line   = b;
        updateCursorIndex(false);
    }
    if (w->wasShortcutTriggered("redo"))
    {
        data_source->stepForwardHistory();
        updateLines();
        auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
        cursor_column = a;
        cursor_line   = b;
        updateCursorIndex(false);
    }
    float lines_tall = size.y / line_height;
    scroll           = glm::clamp(scroll, 0.0f,
        line_height * glm::max(static_cast<float>(lines.size()) - glm::max(lines_tall - 8.0f, 1.0f), 0.0f));
}

void UITextEditor::refresh()
{
    cursor_index              = 0;
    selection_other_end_index = 0;
    cursor_column             = 0;
    cursor_line               = 0;
    scroll                    = 0;
    updateLines();
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
}

std::pair<size_t, size_t> UITextEditor::calculateColumnLineFromIndex(size_t index)
{
    for (size_t l = 0; l < lines.size(); ++l)
    {
        auto line = lines[l];
        if (l + 1 < lines.size())
        {
            auto next_line = lines[l + 1];
            if (index >= line.start && index < line.end) { return { index - line.start, l }; }
            else if (index == line.end && index < next_line.start) { return { index - line.start, l }; }
        }
        else
        {
            if (index >= line.start && index <= line.end) { return { index - line.start, l }; }
        }
    }
    return { 0, lines.size() - 1 };
}

void UITextEditor::eraseSelection()
{
    if (cursor_index == selection_other_end_index) return;
    size_t min = glm::min(cursor_index, selection_other_end_index);
    size_t max = glm::max(cursor_index, selection_other_end_index);
    data_source->getData().erase(min, max - min);
    cursor_index = min;
    updateLines();
    auto [a, b]   = calculateColumnLineFromIndex(cursor_index);
    cursor_column = a;
    cursor_line   = b;
    updateCursorIndex(false);
    scrollCursorOnscreen();
}

void UITextEditor::updateCursorIndex(bool keep_selection)
{
    if (cursor_line < lines.size())
    {
        size_t line_length             = lines[cursor_line].end - lines[cursor_line].start;
        size_t line_start              = lines[cursor_line].start;
        size_t effective_cursor_column = glm::min(line_length, cursor_column);
        cursor_index                   = line_start + effective_cursor_column;
    }
    else
        cursor_index = data_source->getData().size();

    if (!keep_selection) selection_other_end_index = cursor_index;
}

glm::vec2 UITextEditor::findCursorOffset(size_t col, size_t row)
{
    size_t line_length = 0;
    if (row < lines.size()) line_length = lines[row].end - lines[row].start;
    size_t effective_cursor_column = glm::min(line_length, col);
    float char_width               = custom_r->calculateTextWidth("a", format);
    return glm::vec2{ (static_cast<float>(effective_cursor_column) * (char_width + format.spacing)) -
                          format.spacing,
        static_cast<float>(row) * line_height };
}

std::pair<size_t, size_t> UITextEditor::findCursorPlacement(glm::vec2 offset)
{
    float char_width               = custom_r->calculateTextWidth("a", format);
    const glm::vec2 text_start     = position + glm::vec2{ left_margin, (spacing * 2) - scroll };
    glm::vec2 position_within_text = offset - text_start;
    size_t line                    = static_cast<size_t>(glm::floor(position_within_text.y / line_height));
    if (line >= lines.size() && !lines.empty() &&
        (lines[lines.size() - 1].end - lines[lines.size() - 1].start) > 1)
    {
        data_source->getData().push_back('\n');
        updateLines();
    }
    if (line >= lines.size()) line = lines.size() - 1;
    size_t col = static_cast<float>(glm::max(0.0f,
        glm::round((position_within_text.x + format.spacing) / (char_width + format.spacing))));
    return { col, line };
}

std::pair<size_t, size_t> UITextEditor::collectParagraph(size_t index)
{
    size_t start = cursor_index;
    size_t end   = cursor_index;
    while (start > 0 && data_source->getData()[start - 1] != '\n') --start;
    while (end < data_source->getData().size() && data_source->getData()[end] != '\n') ++end;
    return { start, end };
}

float UITextEditor::findIndexOffset(size_t index, size_t line_start)
{
    float char_width = custom_r->calculateTextWidth("a", format);
    return (static_cast<float>(index - line_start) * (char_width + format.spacing)) - format.spacing;
}

void UITextEditor::scrollCursorOnscreen()
{
    size_t top_line    = static_cast<size_t>(scroll / line_height);
    size_t bottom_line = static_cast<size_t>((scroll + size.y) / line_height);
    if (cursor_line < top_line) { scroll -= line_height * static_cast<float>(top_line - cursor_line); }
    else if (cursor_line >= bottom_line)
    {
        scroll += line_height * static_cast<float>((cursor_line - bottom_line) + 1);
    }
}

float UITextEditor::getContentWidth() { return size.x - (left_margin + right_margin); }
