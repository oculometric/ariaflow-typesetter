#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace BBUI
{

class Window;

struct Texture final
{
    const unsigned char* data;
    const size_t size;
};

struct Font final
{
    Texture regular_atlas;
    Texture bold_atlas;
    glm::vec2 glyph_size;
};

struct Vertex final
{
    glm::vec3 position;
    glm::vec4 colour_1;
    glm::vec4 colour_2;
    glm::vec4 data_1;
    glm::vec4 data_2;
    glm::vec2 uv;
};

typedef unsigned int Index;

class Backend
{
public:
    Backend(Font font, Texture slice_atlas, Texture icon_atlas) {}
    Backend()                            = default;
    Backend(const Backend& other)        = delete;
    Backend(Backend&& other)             = delete;
    void operator=(const Backend& other) = delete;
    void operator=(Backend&& other)      = delete;
    virtual ~Backend()                   = default;

    virtual void mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices) {};
    virtual void bind(std::shared_ptr<Window> window) const {};
    virtual void draw(std::shared_ptr<Window> window) const {};
};

class Backend_OpenGL final : Backend
{
private:
    typedef unsigned int Handle;

    Handle vertex_buffer       = 0;
    Handle index_buffer        = 0;
    Handle vertex_array_object = 0;
    unsigned int index_count   = 0;

    Handle shader_program = 0;

    Handle text_atlas_texture      = 0;
    Handle text_bold_atlas_texture = 0;
    Handle slice_atlas_texture     = 0;
    Handle icon_atlas_texture      = 0;

public:
    Backend_OpenGL(Font font, Texture slice_atlas, Texture icon_atlas);
    Backend_OpenGL()                            = delete;
    Backend_OpenGL(const Backend_OpenGL& other) = delete;
    Backend_OpenGL(Backend_OpenGL&& other)      = delete;
    void operator=(const Backend_OpenGL& other) = delete;
    void operator=(Backend_OpenGL&& other)      = delete;
    ~Backend_OpenGL() override;

    void mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices) override;
    void bind(std::shared_ptr<Window> window) const override;
    void draw(std::shared_ptr<Window> window) const override;
};

class Renderer final : std::enable_shared_from_this<Renderer>
{
    friend class Backing;

public:
    struct Backing final
    {
    private:
        uint32_t source_id = 0;
        Index vertex_start;
        Index index_start;
        Index quad_count;

    public:
        void ensure(Index capacity);
        void write(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 uv_tl, glm::vec2 uv_br,
            glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2, Index offset = 0);
        void release();
    };

    class Object
    {
        friend class Renderer;

    protected:
        glm::vec3 position = { 0, 0, 0 };
        glm::vec2 size     = { 10, 10 };
        glm::vec4 colour_a = { 1, 1, 1, 1 };
        glm::vec4 colour_b = { 0, 0, 0, 0 };

    private:
        std::weak_ptr<Renderer> renderer;
        Backing backing;

    public:
        virtual ~Object() {};

        void setPosition(glm::vec3 _position);
        void setSize(glm::vec2 _size);

    protected:
        virtual void update();

    private:
        Object()                            = default;
        Object(const Object& other)         = delete;
        Object(Object&& other)              = delete;
        void operator=(const Object& other) = delete;
        void operator=(Object&& other)      = delete;
    };

    class Text final : public Object
    {
    public:
        enum Align : uint8_t
        {
            TEXT_ALIGN_LEFT,
            TEXT_ALIGN_CENTER,
            TEXT_ALIGN_RIGHT
        };

        enum Flags : uint8_t
        {
            TEXT_FLAGS_NONE          = 0b000000,
            TEXT_FLAGS_BOLD          = 0b000001,
            TEXT_FLAGS_ITALIC        = 0b000010,
            TEXT_FLAGS_UNDERLINE     = 0b000100,
            TEXT_FLAGS_STRIKETHROUGH = 0b001000,
            TEXT_FLAGS_WRAP          = 0b010000,
            TEXT_FLAGS_CLIP          = 0b100000
        };

        struct Format final
        {
            Align align = TEXT_ALIGN_LEFT;
            Flags flags = TEXT_FLAGS_NONE;
            int spacing = 1;
            float size  = 24;
        };

    protected:
        std::string content;
        Format format;

    public:
        virtual ~Text() {};

        void setForeground(glm::vec4 _colour);
        void setBackground(glm::vec4 _colour);
        void setText(const std::string& _content);
        void setFormat(const Format& _format);

    protected:
        virtual void update();
    };

    class NineSlice final : public Object
    {
    public:
        enum Borders : uint8_t
        {
            NS_BORDERS_ALL        = 0b1111,
            NS_BORDERS_NONE       = 0b0000,
            NS_BORDERS_TOP        = 0b0001,
            NS_BORDERS_BOTTOM     = 0b0010,
            NS_BORDERS_HORIZONTAL = 0b0011,
            NS_BORDERS_LEFT       = 0b0100,
            NS_BORDERS_RIGHT      = 0b1000,
            NS_BORDERS_VERTICAL   = 0b1100,
        };

    protected:
        int pattern_index;
        Borders borders;

    public:
        virtual ~NineSlice() {};

        void setFill(glm::vec4 _colour);
        void setBorder(glm::vec4 _colour);
        void setPattern(int _pattern_index);
        void setBorders(Borders _borders);

    protected:
        virtual void update();
    };

    class Icon final : public Object
    {
    protected:
        int icon_index;

    public:
        virtual ~Icon() {};

        void setForeground(glm::vec4 _colour);
        void setBackground(glm::vec4 _colour);
        void setIconIndex(int _icon_index);

    protected:
        virtual void update();
    };

    class Quad final : public Object
    {
    protected:
        glm::vec2 uv_top_left;
        glm::vec2 uv_bottom_right;
        glm::vec4 data_1;
        glm::vec4 data_2;

    public:
        virtual ~Quad() {};

        void setColourA(glm::vec4 _colour_a);
        void setColourB(glm::vec4 _colour_b);
        void setUVTopLeft(glm::vec2 _uv_top_left);
        void setUVBottomRight(glm::vec2 _uv_bottom_right);
        void setData1(glm::vec4 _data_1);
        void setData2(glm::vec4 _data_2);

    protected:
        virtual void update();
    };

private:
    std::unique_ptr<Backend> backend;

    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    const size_t max_dead_quads = 256;
    size_t dead_quads           = 0;
    bool source_modified        = false;
    uint32_t current_source_id  = 0;

public:
    glm::mat3 transform = glm::mat3(1.0);

public:
    Renderer();
    Renderer(const Renderer& other)       = delete;
    Renderer(Renderer&& other)            = delete;
    void operator=(const Renderer& other) = delete;
    void operator=(Renderer&& other)      = delete;
    ~Renderer();

    std::shared_ptr<Text> createText();
    std::shared_ptr<NineSlice> createNineSlice();
    std::shared_ptr<Icon> createIcon();
    std::shared_ptr<Quad> createQuad();

    void draw(std::shared_ptr<Window> window) const;
};

// TODO: ability to configure whether we use blended alpha or dithered alpha?
// TODO: ability to configure whether we use linear or nearest interpolation?

}; // namespace BBUI
