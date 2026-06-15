#pragma once

#include "window.h"

#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

namespace AriaFlow
{

class Window;

struct Vertex
{
    glm::vec3 position;
    glm::vec4 colour_1;
    glm::vec4 colour_2;
    glm::vec4 data_1;
    glm::vec4 data_2;
    glm::vec2 uv;
};

enum TextAlign : uint8_t
{
    TEXT_ALIGN_LEFT   = 0,
    TEXT_ALIGN_CENTER = 1,
    TEXT_ALIGN_RIGHT  = 2
};

enum TextFlags : uint8_t
{
    TEXT_FLAGS_NONE          = 0b0000,
    TEXT_FLAGS_BOLD          = 0b0001,
    TEXT_FLAGS_ITALIC        = 0b0010,
    TEXT_FLAGS_UNDERLINE     = 0b0100,
    TEXT_FLAGS_STRIKETHROUGH = 0b1000
};

struct TextFormatting
{
    TextAlign align           = TEXT_ALIGN_LEFT;
    TextFlags flags           = TEXT_FLAGS_NONE;
    bool terminate_at_newline = false;
    bool wrap                 = false;
    bool clip                 = false;
    glm::ivec2 clip_bounds    = { 0, 0 };
    int spacing               = 0;
    float size                = 24;
};

class UIRenderer
{
public:
    struct BackingData
    {
        friend class UIRenderer;

    private:
        uint32_t id = 0;
    };

private:
    struct BackingDataInternal
    {
        uint16_t first_vertex;
        uint16_t vertex_count;

        uint16_t first_index;
        uint16_t index_count;
        float z;
    };

private:
    unsigned int vertex_buffer           = 0;
    unsigned int index_buffer            = 0;
    unsigned int vertex_array_object     = 0;
    unsigned int index_count             = 0;
    unsigned int shader_program          = 0;
    unsigned int transform_var           = 0;
    unsigned int text_atlas_texture      = 0;
    unsigned int text_bold_atlas_texture = 0;
    unsigned int slice_atlas_texture     = 0;
    unsigned int icon_atlas_texture      = 0;
    unsigned int line_atlas_texture      = 0;

    std::vector<Vertex> vertices;
    std::map<float, std::vector<unsigned int>> indices;
    std::map<uint32_t, BackingDataInternal> backing_datas;
    uint32_t next_id    = 0;
    glm::mat3 transform = glm::mat3(1.0);

    glm::vec2 text_size         = { 34, 62 };
    glm::vec4 background_colour = { 0, 0, 0, 0 };

public:
    UIRenderer();
    UIRenderer(const UIRenderer& other)     = delete;
    UIRenderer(UIRenderer&& other)          = delete;
    void operator=(const UIRenderer& other) = delete;
    void operator=(UIRenderer&& other)      = delete;
    ~UIRenderer();

    void clear();
    void addQuad(float z, BackingData& backing_ref);
    void addQuad(float z);
    void addQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float z, glm::vec2 uv_tl,
        glm::vec2 uv_br, glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2,
        BackingData& backing_ref);
    void addQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float z, glm::vec2 uv_tl,
        glm::vec2 uv_br, glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2);
    glm::vec2 addText(glm::vec2 position, float z, TextFormatting formatting, const std::string& text,
        glm::vec3 colour, BackingData& backing);
    glm::vec2 addText(glm::vec2 position, float z, TextFormatting formatting, const std::string& text,
        glm::vec3 colour);
    void addNineSlice(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec4 fill,
        uint8_t borders, BackingData& backing);
    void addNineSlice(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec4 fill,
        uint8_t borders);
    void addSimple(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec2 uv_base,
        glm::vec2 uv_size, BackingData& backing);
    void addSimple(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec2 uv_base,
        glm::vec2 uv_size);
    void finalise();

    float calculateTextWidth(const std::string& text, TextFormatting formatting);
    void setTransformation(glm::mat3 transform_matrix) { transform = transform_matrix; }

    void draw(Window* window) const;

private:
    bool isBackingValid(const BackingData& backing_ref);
    void addBacking(BackingData& backing_ref, BackingDataInternal backing);
    void updateTextSingleLine(glm::vec2 position, TextFormatting formatting, const std::string& text,
        glm::vec3 colour, BackingDataInternal backing);
    void updateQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 uv_tl,
        glm::vec2 uv_br, glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2,
        BackingDataInternal backing);
};

class UIMenu
{
private:
    struct Item
    {
        std::string text;
        std::string shortcut;
        bool is_clickable = false;
        bool is_divider   = false;
        bool is_submenu   = false;
        std::function<void(void)> callback;
        UIMenu* submenu = nullptr;
        int icon        = -1;
        glm::vec2 position;
        glm::vec2 size;
        bool is_clicked = false;
    };

    std::vector<Item> items;
    glm::vec2 overall_size;

public:
    UIMenu();
    UIMenu(const UIMenu& other)         = delete;
    UIMenu(UIMenu&& other)              = delete;
    void operator=(const UIMenu& other) = delete;
    void operator=(UIMenu&& other)      = delete;
    ~UIMenu();

    void addButton(const std::string& text, std::function<void(void)> callback,
        const std::string& shortcut = "", int icon = -1);
    void addLabel(const std::string& text, int icon = -1);
    void addDivider();
    UIMenu* addSubMenu(const std::string& text, int icon = -1);

    void draw(UIRenderer* r, glm::vec2 top_left);
    bool checkInput(Window* w, glm::vec2 top_left);
};

class UIRootMenu
{
private:
    struct Item
    {
        std::string text;
        bool is_clickable = false;
        bool is_submenu   = false;
        std::function<void(void)> callback;
        UIMenu* submenu = nullptr;
        int icon        = -1;
        glm::vec2 position;
        glm::vec2 size;
        bool is_clicked = false;
    };

    std::vector<Item> items;
    bool is_menu_open = false;
    bool release_flag = false;

public:
    UIRootMenu();
    UIRootMenu(const UIRootMenu& other)     = delete;
    UIRootMenu(UIRootMenu&& other)          = delete;
    void operator=(const UIRootMenu& other) = delete;
    void operator=(UIRootMenu&& other)      = delete;
    ~UIRootMenu();

    void addButton(const std::string& text, std::function<void(void)> callback, int icon = -1);
    void addLabel(const std::string& text, int icon = -1);
    UIMenu* addSubMenu(const std::string& text, int icon = -1);

    static float getHeight();

    void draw(UIRenderer* r, float width);
    void checkInput(Window* w);
};

class UIButton
{
private:
    std::string message;
    int icon_index;
    glm::vec2 last_size;
    bool is_pressed = false;
    std::function<void(void)> callback_func;
    bool mouse_inside = false;

public:
    UIButton(const std::string& text, std::function<void(void)> callback, int icon = -1);
    UIButton(const UIButton& other)       = delete;
    UIButton(UIButton&& other)            = delete;
    void operator=(const UIButton& other) = delete;
    void operator=(UIButton&& other)      = delete;
    ~UIButton() {};

    glm::vec2 getSize(UIRenderer* r);

    void draw(UIRenderer* r, glm::vec2 position);
    void checkInput(Window* w, glm::vec2 position);
};

class UILabel
{
private:
    std::string message;
    int icon_index;
    TextAlign direction;
    TextFlags settings;

public:
    UILabel(const std::string& text, TextAlign align, TextFlags flags, int icon = -1);
    UILabel(const UILabel& other)        = delete;
    UILabel(UILabel&& other)             = delete;
    void operator=(const UILabel& other) = delete;
    void operator=(UILabel&& other)      = delete;
    ~UILabel() {};

    void draw(UIRenderer* r, glm::vec2 position);
};

class UIPanel
{
private:
    glm::vec4 fill_colour;
    int layer_index;
    uint8_t border_flags;

public:
    UIPanel(glm::vec4 fill, int layer, uint8_t borders);
    UIPanel(const UIPanel& other)        = delete;
    UIPanel(UIPanel&& other)             = delete;
    void operator=(const UIPanel& other) = delete;
    void operator=(UIPanel&& other)      = delete;
    ~UIPanel() {};

    void draw(UIRenderer* r, glm::vec2 position, glm::vec2 size);
};

class UIGrabbable
{
private:
    bool grabbed;
    CursorType cursor;

public:
    UIGrabbable(CursorType cursor_indicator = CURSOR_NORMAL);
    UIGrabbable(const UIGrabbable& other)    = delete;
    UIGrabbable(UIGrabbable&& other)         = delete;
    void operator=(const UIGrabbable& other) = delete;
    void operator=(UIGrabbable&& other)      = delete;
    ~UIGrabbable() {};

    bool isCurrentlyGrabbed() const { return grabbed; }

    glm::vec2 checkInput(Window* w, glm::vec2 position, glm::vec2 area_size);
};

class UIButtonPalette
{
public:
    glm::vec2 position;
    glm::vec2 size;

private:
    UIPanel* panel;
    std::array<UIGrabbable*, 3> grabbables;
    std::vector<UIButton*> buttons;
    int columns = 2;
    glm::vec2 button_size;

public:
    UIButtonPalette();
    UIButtonPalette(const UIButtonPalette& other) = delete;
    UIButtonPalette(UIButtonPalette&& other)      = delete;
    void operator=(const UIButtonPalette& other)  = delete;
    void operator=(UIButtonPalette&& other)       = delete;
    ~UIButtonPalette();

    UIButton* addButton(int icon, std::function<void(void)> callback);

    void draw(UIRenderer* r);
    void checkInput(Window* w);

private:
    glm::vec2 recalculateSize();
    glm::vec2 calculateButtonArea();
};

class UITextEditor
{
public:
    glm::vec2 position;
    glm::vec2 size;

private:
    std::string text;
    std::vector<std::pair<size_t, size_t>> lines;
    size_t cursor_index  = 0;
    size_t cursor_line   = 0;
    size_t cursor_column = 0;
    glm::vec2 last_checked_size;
    bool needs_lines_update;
    bool needs_cursor_update;
    std::array<UIGrabbable*, 9> grabbables;

public:
    UITextEditor(glm::vec2 offset = { 0, 0 }, glm::vec2 dimensions = { 16, 16 });
    UITextEditor(const UITextEditor& other)   = delete;
    UITextEditor(UITextEditor&& other)        = delete;
    void operator=(const UITextEditor& other) = delete;
    void operator=(UITextEditor&& other)      = delete;
    ~UITextEditor();

    glm::ivec2 getCursorPos() const { return { cursor_line + 1, cursor_column + 1 }; }
    std::string getContent() const { return text; }
    void setContent(const std::string& content, size_t cursor_index)
    {
        text = content;
        updateLines();
        updateCursor();
    }

    void draw(UIRenderer* r);
    void checkInput(Window* w);

private:
    void updateLines();
    void updateCursor();
};

bool insideRect(glm::vec2 point, glm::vec2 top_left, glm::vec2 size);
bool checkForMouseDown(Window* w);
bool checkForMouseUp(Window* w);
void consumeAllMouseEvents(Window* w);
void trackWindowResize(Window* w, glm::vec2& top_left, glm::vec2 size);

const float line_height          = 24.0f;
const float icon_size            = 24.0f;
const float spacing              = 2.0f;
const float text_push            = -2.0f;
const glm::vec4 panel_colour     = { 0.12f, 0.12f, 0.12f, 1.0f };
const glm::vec4 panel_sec_colour = { 0.3f, 0.3f, 0.3f, 1.0f };
const glm::vec3 text_colour      = { 0.9f, 0.9f, 0.9f };
const glm::vec3 text_sec_colour  = { 0.5f, 0.5f, 0.5f };

}; // namespace AriaFlow