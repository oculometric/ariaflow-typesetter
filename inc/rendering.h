#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace GLUI
{

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
    TextAlign align = TEXT_ALIGN_LEFT;
    TextFlags flags = TEXT_FLAGS_NONE;
    bool wrap       = false;
    bool clip       = false;
    int spacing     = 1;
    float size      = 24;
};

class Renderer
{
public:
    struct Backing
    {
        friend class Renderer;

    private:
        uint32_t id = 0;
    };

private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec4 colour_1;
        glm::vec4 colour_2;
        glm::vec4 data_1;
        glm::vec4 data_2;
        glm::vec2 uv;
    };

    typedef unsigned int Index;
    typedef unsigned int Handle;

    struct BackingDataInternal
    {
        Index vertex_start;
        Index index_start;
        Index quad_count;
    };

private:
    Handle vertex_buffer           = 0;
    Handle index_buffer            = 0;
    Handle vertex_array_object     = 0;
    unsigned int index_count       = 0;
    Handle shader_program          = 0;
    Handle text_atlas_texture      = 0;
    Handle text_bold_atlas_texture = 0;
    Handle slice_atlas_texture     = 0;
    Handle icon_atlas_texture      = 0;

    const size_t max_dead_quads;

    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    size_t dead_quads;
    bool needs_commit;

    std::map<uint32_t, BackingDataInternal> backing_datas;
    uint32_t next_backing_id = 0;

public:
    glm::mat3 transform = glm::mat3(1.0);

public:
    Renderer(); // TODO: control over textures used for icons, nineslice, text (multiple fonts)
    Renderer(const Renderer& other)       = delete;
    Renderer(Renderer&& other)            = delete;
    void operator=(const Renderer& other) = delete;
    void operator=(Renderer&& other)      = delete;
    ~Renderer();

    glm::vec2 addText(Backing& backing, glm::vec3 position, glm::vec2 size, TextFormatting formatting,
        const std::string& text, glm::vec4 fg_colour, glm::vec4 bg_colour);
    void addNineSlice(Backing& backing, glm::vec3 position, glm::vec2 size, glm::vec4 fill_colour,
        glm::vec4 border_colour, int pattern_index, uint8_t borders);
    void addIcon(Backing& backing, glm::vec3 position, glm::vec2 size, glm::vec4 fg_colour,
        glm::vec4 bg_colour, int icon_index);
    void addImage(Backing& backing, glm::vec3 position, glm::vec2 size, glm::vec4 mult_colour,
        int image_index);
    void remove(Backing& backing);

    float calculateTextWidth(const std::string& text, TextFormatting formatting) const;

    void draw(std::shared_ptr<Window> window)
        const; // commit the vertex and index buffers to the gpu (if needed), prepare the drawing
               // environment, draw the geometry, if there are too many dead quads then clear all the
               // backing data

private:
    void initEnvironment(); // prepare graphics resources (vertex buffer, index buffer, array object,
                            // shader, textures)

    void checkBacking(Backing& backing_ref,
        uint16_t count); // check if the backing ref is valid and if the size is correct, and
                         // create/recreate it if needed (if needs larger, then recreate, if needs smaller
                         // then just clear the extra quads)
    void updateQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 uv_tl,
        glm::vec2 uv_br, glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2,
        Backing& backing,
        uint16_t offset); // update the actual data for a given quad (identified by backing and offset) (set
                          // needs_commit if the data is different)
};

}; // namespace GLUI

/*
what if instead, the renderer can allocate objects which kinda like smart pointers, represent requirements
for the renderer so they can be modified by the owner (reference count is kept), and those modifications
propagate back to the renderer when draw is called (i.e. we check if any changes were made, and if so then
we update the geometry).
so the user has an object which contains some attributes (position, size, colours, and more which get
transmuted into the underlying data according to functions depending upon the object type)
and the renderer knows about those objects primarily - the backing data is linked to them, but it can be
mutable without having to redraw everything.
ui elements, when created, allocate things they want (an icon, some text), and can then modify them at any
time.
those modifications set flags within the object that the underlying data needs to be updated.

// TODO: ability to configure whether we use blended alpha or dithered alpha?

RENDERER (friend BACKING)
  (vertices)
  (indices)
  (dead quad count)
  (current array id)
  (modified)

  -> create TEXT
  -> create QUAD
  -> create NINESLICE
  -> create ICON
  -> create IMAGE

  -> construct : init graphics
  -> draw : update mesh arrays if modified

BACKING
  (vertices start)
  (indices start)
  (quad count)
  (array id)

  -> ensure backing validity and size
  -> write data to arrays on renderer
  -> release backing

TEXT_T
  position
  size
  colour a
  colour b
  text

  (renderer)
  (backing)

  (-> construct : update)
  -> update : ensure backing validity, write data to backing, set modified
  -> destruct : release backing, set modified
  -> set position : update
  -> set size : update
  -> set colour a : update
  -> set colour b : update
  -> set text : update

TEXT : std::shared_ptr<TEXT_T>
*/