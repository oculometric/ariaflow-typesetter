#include "document.h"

#include <filesystem>
#include <format>
#include <fstream>

using namespace AriaFlow;

const char* preset_demo_document =
    R"VOGON(@DocumentConfig[figure.captionstyle=italic; font="Courier New"; list.bullet=round] // etc, default format options for various blocks

This is some plain text. It will be @I{formatted} according to the default formatting defined in the
@B{DocumentConfig}. I want to make sure that my figure is on the same page as @Marker[id="here"]{this} block of text.

@LineBreak

@Figure[tag="my-figure"]{
    @Restrict[arrange=samepage & below; marker="here"]
    @Embed[source="figure.png"]
    This is the caption.
}

I would like to make mention of @Mention[figure="my-figure"], or as it is also known, @Mention[figure=last].

@PageBreak

@Format[columns=2, prefer=equal]{
    This paragraph will be split into two columns.
    @Embed[source="figure2.png"]
    @Figure[tag="single-col-figure"]{@Embed[source="table.csv"]This figure will only cover one column, but it will contain a table!}
    @Code[highlighter=c++]{
        #include <iostream>
        int main()
        {
            std::cout << "Hello, World!" << std::endl;
            return 0;
        }
    }
    @Figure[tag="double-column-figure",columns=1]{@Embed[source="test.svg"]This figure will span both columns.}
}

@List[bullet=round]{first item}
@List[bullet=round]{second item}
@List[bullet=round]{third item}
@List[bullet=round]{fourth item})VOGON";

Document::Document()
{
    path = "";
    revert();
    has_unsaved_changes = true;
}

Document::Document(const std::string& load_path)
{
    path = load_path;
    revert();
}

bool Document::save()
{
    if (path.empty()) return false;

    std::ofstream file(path);
    if (!file.is_open()) return false;
    file.write(data.data(), data.size());
    file.close();
    has_unsaved_changes = false;
    return true;
}

bool Document::saveAs(const std::string& new_path)
{
    if (new_path.empty()) return false;
    path = new_path;

    return save();
}

bool Document::saveIncremental()
{
    if (path.empty()) return false;

    // trim off the extension
    auto p                   = std::filesystem::path(path);
    std::string trimmed_path = (p.parent_path().append(p.stem().generic_string())).generic_string();

    // first check if the last character of the path is a number, and track backwards until it isnt
    size_t index = trimmed_path.size() - 1;
    while (trimmed_path[index] >= '0' && trimmed_path[index] <= '9' && index > 0) --index;
    size_t chars_in_number = trimmed_path.size() - (index + 1);
    size_t chars_in_name   = index - 1;

    std::string new_path;
    if (chars_in_name > 0 && chars_in_number > 0)
    {
        // if we find a regular letter before that, then we can extract the number and increment it
        size_t version = std::stoi(trimmed_path.substr(index + 1));
        new_path       = trimmed_path.substr(0, index + 1) + std::format("{:0>4}", version + 1);
    }
    else
    {
        // otherwise we have to append "_v0001" to the path (which should be parsed correctly next time)
        new_path = trimmed_path + "_v0001";
    }
    new_path += p.extension().generic_string();

    return saveAs(new_path);
}

void Document::revert()
{
    if (path.empty())
    {
        data = preset_demo_document;
        return;
    }

    std::ifstream file(path, std::ios::ate);
    if (!file.is_open())
    {
        data = preset_demo_document;
        return;
    }
    size_t size = file.tellg();
    file.seekg(std::ios::beg);
    data.resize(size);
    file.read(data.data(), data.size());
    file.close();

    for (size_t i = 0; i < data.size(); ++i)
    {
        if (data[i] == '\r')
        {
            if (i + 1 < data.size() && data[i + 1] == '\n')
            {
                data.erase(i);
                --i;
                continue;
            }
            else
            {
                data[i] = '\n';
                continue;
            }
        }
        if (data[i] == '\t')
        {
            data[i] = ' ';
            data.insert(i, "   ");
            i += 3;
        }
    }

    has_unsaved_changes = false;
}

bool isBreakable(char c)
{
    if (c >= 'a' && c <= 'z') return false;
    if (c >= 'A' && c <= 'Z') return false;
    if (c >= '0' && c <= '9') return false;
    return true;
}

bool isWhitespace(char c)
{
    if (c == ' ' || c == '\t' || c == '\n') return true;
    return false;
}

std::vector<std::pair<size_t, size_t>> Document::splitToLines(size_t chars_per_line) const
{
    size_t current    = 0;
    size_t line_start = 0;
    std::vector<std::pair<size_t, size_t>> lines;

    while (current < data.size())
    {
        if (data[current] == '\n')
        {
            size_t line_end = current;
            lines.emplace_back(line_start, line_end);
            ++current;
            line_start = current;
            continue;
        }
        if (current - line_start == chars_per_line)
        {
            size_t line_end = current;
            while (current > line_start && !isBreakable(data[current])) { --current; }
            if (current == line_start) current = line_end;
            else
                line_end = current;
            lines.emplace_back(line_start, line_end);
            if (isWhitespace(data[current])) ++current;
            line_start = current;
            continue;
        }
        ++current;
    }
    lines.emplace_back(line_start, current);

    return lines;
}

int Document::getCharacterType(size_t index) const
{
    char c = data[index];
    if (c >= 'a' && c <= 'z') return 0;
    if (c >= 'A' && c <= 'Z') return 0;
    if (c >= '0' && c <= '9') return 0;
    if (c == '@') return 0;
    if (c == ' ' || c == '\t' || c == '\n') return 1;
    return 2;
}

size_t Document::findNextWord(size_t current) const
{
    size_t initial   = current;
    int current_type = getCharacterType(current);
    while ((current < data.size()) &&
           ((getCharacterType(current) == current_type) || (getCharacterType(current) == 1)))
    {
        if (getCharacterType(current) == 1) current_type = 1;
        if (!(initial == current) && current + 1 < data.size() && data[current] == '\n' &&
            data[current + 1] == '\n')
            break;
        else if (initial == current && data[current] == '\n')
        {
            ++current;
            break;
        }
        ++current;
    }
    return current;
}

size_t Document::findPrevWord(size_t current) const
{
    if (current <= 1) return 0;

    size_t initial   = current;
    int current_type = getCharacterType(current);
    if (getCharacterType(current - 1) != getCharacterType(current) && getCharacterType(current - 1) != 1)
    {
        --current;
        current_type = getCharacterType(current);
    }
    else if (getCharacterType(current - 1) == 1)
        --current;
    while (current > 0)
    {
        if (getCharacterType(current - 1) != current_type && getCharacterType(current) != 1)
        {
            if (current + 1 < data.size() && data[current + 1] == '\n') ++current;
            break;
        }
        if (!(initial == current) && data[current] == '\n' && data[current - 1] == '\n') break;
        --current;
    }
    return current;
}

void Document::pushHistory()
{
    if (history_steps_back != 0)
    {
        history.erase(history.end() - history_steps_back, history.end());
        history_steps_back = 0;
    }
    else
    {
        history.push_back(data);
        if (history.size() > 256) history.erase(history.begin());
    }
    setUnsavedFlag();
}

void Document::stepBackHistory()
{
    if (history.empty()) return;

    if (history_steps_back == 0) history.push_back(data);

    if (history_steps_back + 1 < history.size())
    {
        ++history_steps_back;
        data = *(history.rbegin() + history_steps_back);
    }
}

void Document::stepForwardHistory()
{
    if (history.empty() || history_steps_back == 0) return;

    --history_steps_back;
    data = *(history.rbegin() + history_steps_back);
    if (history_steps_back == 0) { history.pop_back(); }
}

bool Document::hasUndoStepsAvailable()
{
    if (history.empty()) return false;
    if (history_steps_back >= history.size()) return false;
    return true;
}

bool Document::hasRedoStepsAvailable()
{
    if (history.empty()) return false;
    if (history_steps_back == 0) return false;
    return true;
}
