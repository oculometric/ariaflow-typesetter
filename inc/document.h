#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace AriaFlow
{

class Document
{
private:
    std::string path;
    std::string data;
    bool has_unsaved_changes = true;

public:
    Document();
    Document(const std::string& load_path);
    Document(const Document& other)       = delete;
    Document(Document&& other)            = delete;
    void operator=(const Document& other) = delete;
    void operator=(Document&& other)      = delete;
    ~Document() {};

    std::string& getData() { return data; }
    std::string getPath() const { return path; }
    bool hasUnsavedChanges() const { return has_unsaved_changes; }

    void setUnsavedFlag() { has_unsaved_changes = true; }
    bool save();
    bool saveAs(const std::string& new_path);
    void saveIncremental();
    void revert();

    std::vector<std::pair<size_t, size_t>> splitToLines(size_t chars_per_line) const;
    std::vector<uint32_t> parse() const;
    void format();

    size_t findNextWord(size_t current) const;
    size_t findPrevWord(size_t current) const;

private:
    int getCharacterType(size_t index) const;
};

} // namespace AriaFlow