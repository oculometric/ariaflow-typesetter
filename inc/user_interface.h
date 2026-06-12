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
    // TODO: styling, material stuff
    unsigned int vertex_buffer       = 0;
    unsigned int index_buffer        = 0;
    unsigned int vertex_array_object = 0;
    unsigned int index_count         = 0;
    unsigned int shader_program      = 0;
    unsigned int transform_var       = 0;
    std::vector<Vertex> vertices;
    std::map<float, std::vector<uint16_t>> indices;
    std::map<uint32_t, BackingDataInternal> backing_datas;
    uint32_t next_id    = 0;
    glm::mat3 transform = glm::mat3(1.0);

public:
    UIRenderer();
    ~UIRenderer();

    void clear();
    void addQuad(float z, BackingData& backing_ref);
    void addQuad(float z);
    void addQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float z, glm::vec2 uv_tl,
        glm::vec2 uv_br, glm::vec4 colour, glm::vec4 normal, glm::vec4 tangent, BackingData& backing_ref);
    void addQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float z, glm::vec2 uv_tl,
        glm::vec2 uv_br, glm::vec4 colour, glm::vec4 normal, glm::vec4 tangent);
    glm::vec2 addText(glm::vec2 position, float z, TextFormatting formatting, const std::string& text,
        glm::vec3 colour, BackingData& backing);
    glm::vec2 addText(glm::vec2 position, float z, TextFormatting formatting, const std::string& text,
        glm::vec3 colour);
    void addNineSlice(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec4 fill,
        BackingData& backing);
    void addNineSlice(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec4 fill);
    void addSimple(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec2 uv_base,
        glm::vec2 uv_size, BackingData& backing);
    void addSimple(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec2 uv_base,
        glm::vec2 uv_size);
    void finalise();

    void setTransformation(glm::mat3 transform_matrix) { transform = transform_matrix; }

    void draw(Window* window) const;

private:
    bool isBackingValid(const BackingData& backing_ref);
    void addBacking(BackingData& backing_ref, BackingDataInternal backing);
    void updateTextSingleLine(glm::vec2 position, TextFormatting formatting, const std::string& text,
        glm::vec3 colour, BackingDataInternal backing);
    void updateQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 uv_tl,
        glm::vec2 uv_br, glm::vec4 colour, glm::vec4 normal, glm::vec4 tangent,
        BackingDataInternal backing);
};

}; // namespace AriaFlow