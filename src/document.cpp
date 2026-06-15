#include "document.h"

#include <filesystem>
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
    // path = load_path;
    // has_unsaved_changes = false;
    // TODO: load document from existing file
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
    // TODO: split lines
}
