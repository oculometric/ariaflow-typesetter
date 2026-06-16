#pragma once

#include <cstdint>
#include <deque>
#include <queue>
#include <string>
#include <vector>

template<typename T, typename Container = std::deque<T>> class iterable_queue :
    public std::queue<T, Container>
{
public:
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    iterator begin() { return this->c.begin(); }
    iterator end() { return this->c.end(); }
    const_iterator begin() const { return this->c.begin(); }
    const_iterator end() const { return this->c.end(); }
};

namespace AriaFlow
{

class Document
{
private:
    std::string path;
    std::string data;
    bool has_unsaved_changes = true;

    std::vector<std::string> history;
    size_t history_steps_back = 0;

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

    void pushHistory();
    void stepBackHistory();
    void stepForwardHistory();
    bool hasUndoStepsAvailable();
    bool hasRedoStepsAvailable();

private:
    int getCharacterType(size_t index) const;
};

} // namespace AriaFlow