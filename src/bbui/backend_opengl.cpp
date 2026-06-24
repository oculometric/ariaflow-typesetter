#include "bbui/rendering.h"
#include "bbui/window.h"
#include "rendering.h"

#include <glad.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <stdexcept>

using namespace BBUI;

static const char* vertex_shader_source = R"(
#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_colour_1;
layout (location = 2) in vec4 vertex_colour_2;
layout (location = 3) in vec4 vertex_data_1;
layout (location = 4) in vec4 vertex_data_2;
layout (location = 5) in vec2 vertex_uv;

out vec2 position;
out vec4 colour_1;
out vec4 colour_2;
out vec4 data_1;
out vec4 data_2;
out vec2 uv;

uniform mat4 world_to_clip;

void main()
{
    position = vertex_position.xy;
    colour_1 = vertex_colour_1;
    colour_2 = vertex_colour_2;
    data_1 = vertex_data_1;
    data_2 = vertex_data_2;
    uv = vertex_uv;
    vec4 position = world_to_clip * vec4(vertex_position, 1.0f);
    gl_Position = position;
}
)";

static const char* fragment_shader_source = R"(
#version 330 core

in vec2 position;
in vec4 colour_1;
in vec4 colour_2;
in vec4 data_1;
in vec4 data_2;
in vec2 uv;

out vec4 frag_colour;

uniform sampler2D text_atlas; // 16x16
uniform sampler2D text_bold_atlas; // 16x16
uniform sampler2D slice_atlas; // 2x2
uniform sampler2D icon_atlas; // 8x8
uniform sampler2D line_atlas;

vec2 nineSliceUV(vec2 uv, vec2 quad_size, vec2 atlas_size, bool top_border, bool bottom_border, bool left_border, bool right_border)
{
    vec2 pixels_one_third = atlas_size / 3.0f;
    vec2 pixels_two_third = pixels_one_third * 2.0f;
    vec2 pixel_coord = uv * quad_size;

    // figure out which region of the atlas we should be drawing
    int region_x = 0;
    int region_y = 0;
    if      (pixel_coord.x < pixels_one_third.x)               region_x = 0;
    else if (quad_size.x - pixel_coord.x < pixels_one_third.x) region_x = 2;
    else                                                       region_x = 1;

    if      (pixel_coord.y < pixels_one_third.y)               region_y = 0;
    else if (quad_size.y - pixel_coord.y < pixels_one_third.y) region_y = 2;
    else                                                       region_y = 1;

    // apply border toggles
    if      (region_x == 0 && !left_border)   region_x = 1;
    else if (region_x == 2 && !right_border)  region_x = 1;
    if      (region_y == 0 && !top_border)    region_y = 1;
    else if (region_y == 2 && !bottom_border) region_y = 1;
    
    // figure out our UV within the quadrant
    vec2 new_uv;
    switch (region_x)
    {
        case 0: new_uv.x = min(pixel_coord.x, pixels_one_third.x); break;
        case 1: new_uv.x = pixels_one_third.x + mod(pixel_coord.x, pixels_one_third.x); break;
        case 2: new_uv.x = max((pixel_coord.x - quad_size.x) + atlas_size.x, pixels_two_third.x); break;
    }
    switch (region_y)
    {
        case 0: new_uv.y = min(pixel_coord.y, pixels_one_third.y); break;
        case 1: new_uv.y = pixels_one_third.y + mod(pixel_coord.y, pixels_one_third.y); break;
        case 2: new_uv.y = max((pixel_coord.y - quad_size.y) + atlas_size.y, pixels_two_third.y); break;
    }
    new_uv = new_uv / atlas_size;

    return new_uv;
}

void main()
{
    int draw_mode = int(round(data_1.x));
    vec2 quad_size = data_1.yz;
    ivec2 screen_coord = ivec2(position.xy);

    if (draw_mode == 0)         // text mode
    {
        int segment = int(round(data_1.w));
        int mode = int(round(data_2.x));
        bool bold = bool(mode & 1);
        bool underline = bool(mode & 2);
        bool strikethrough = bool(mode & 4);

        vec2 te_uv = uv;
        vec2 te_off = vec2(float(segment % 16), float((segment / 16) % 16));
        vec2 te_size = quad_size / vec2(textureSize(text_atlas, 0));

        float tex_value = 0.0f;
        if (bold)
            tex_value = texture(text_bold_atlas, (te_uv + te_off) / 16.0f).a;
        else
            tex_value = texture(text_atlas, (te_uv + te_off) / 16.0f).a;
        
        if (underline)
        {
            float underline_start     = 0.1f;
            float underline_end       = 0.2f;
            if (te_uv.y >= underline_start && te_uv.y <= underline_end)
                tex_value = 1.0f;
        }
        if (strikethrough)
        {
            float strikethrough_start = 0.45f;
            float strikethrough_end   = 0.55f;
            if (te_uv.y >= strikethrough_start && te_uv.y <= strikethrough_end)
                tex_value = 1.0f;
        }

        vec4 target_colour = mix(colour_2, colour_1, pow(tex_value, 1.0f));
        if (target_colour.a < 0.1f)
           discard;
        frag_colour = target_colour;
    }
    else if (draw_mode == 1)    // 9-slice mode
    {
        vec2 atlas_size = vec2(textureSize(slice_atlas, 0)) / 2.0f;
        int segment = int(round(data_1.w));
        int borders = int(round(data_2.x));

        vec2 ns_uv = nineSliceUV(uv.xy, quad_size, atlas_size, bool(borders & 1), bool(borders & 2), bool(borders & 4), bool(borders & 8));
        vec2 ns_off = vec2(float(segment % 2), float((segment / 2) % 2));
        
        vec4 tex_colour = texture(slice_atlas, (ns_uv + ns_off) / 2.0f);

        vec4 target_colour = (tex_colour.rgb == vec3(1.0f, 0.0f, 1.0f)) ? colour_1 : (tex_colour * colour_2);
        if (target_colour.a < 0.1f)
            discard;
        frag_colour = target_colour;
    }
    else if (draw_mode == 2)    // icon mode
    {
        vec2 atlas_size = vec2(textureSize(icon_atlas, 0)) / 8.0f;
        int segment = int(round(data_1.w));

        vec2 ic_uv = uv;
        vec2 ic_off = vec2(float(segment % 8), float((segment / 8) % 8));

        vec4 tex_colour = texture(icon_atlas, (ic_uv + ic_off) / 8.0f);

        vec4 target_colour = (tex_colour.a > 0.5f) ? (tex_colour * colour_1) : colour_2;
        if (target_colour.a < 0.1f)
            discard;
        frag_colour = target_colour;
    }
}
)";

Backend_OpenGL::Backend_OpenGL(Font font, Texture slice_atlas, Texture icon_atlas) : Backend()
{
    // construct vertex and index buffers
    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>((offsetof(Vertex, colour_1))));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>((offsetof(Vertex, colour_2))));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>((offsetof(Vertex, data_1))));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>((offsetof(Vertex, data_2))));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>((offsetof(Vertex, uv))));
    glEnableVertexAttribArray(5);

    // construct shader
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    int status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        std::string error;
        error.resize(512);
        glGetShaderInfoLog(vertex_shader, static_cast<int>(error.size()), nullptr,
            const_cast<char*>(error.data()));
        throw std::runtime_error("vertex shader error: " + error);
    }

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        std::string error;
        error.resize(512);
        glGetShaderInfoLog(fragment_shader, static_cast<int>(error.size()), nullptr,
            const_cast<char*>(error.data()));
        throw std::runtime_error("fragment shader error: " + error);
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    if (!status)
    {
        std::string error;
        error.resize(512);
        glGetProgramInfoLog(shader_program, static_cast<int>(error.size()), nullptr,
            const_cast<char*>(error.data()));
        throw std::runtime_error("shader program error: " + error);
    }
    glUniform1i(glGetUniformLocation(shader_program, "text_atlas"), 0);
    glUniform1i(glGetUniformLocation(shader_program, "text_bold_atlas"), 1);
    glUniform1i(glGetUniformLocation(shader_program, "slice_atlas"), 2);
    glUniform1i(glGetUniformLocation(shader_program, "icon_atlas"), 3);
    glUniform1i(glGetUniformLocation(shader_program, "line_atlas"), 4);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // construct textures
    auto loadTexture = [](unsigned int* texture, int slot, const Texture& data) -> void
    {
        glGenTextures(1, texture);
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        int x, y, c;
        stbi_uc* pixels = stbi_load_from_memory(data.data, data.size, &x, &y, &c, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        stbi_image_free(pixels);
    };

    loadTexture(&text_atlas_texture, GL_TEXTURE0, font.regular_atlas);
    loadTexture(&text_bold_atlas_texture, GL_TEXTURE1, font.bold_atlas);
    loadTexture(&slice_atlas_texture, GL_TEXTURE2, slice_atlas);
    loadTexture(&icon_atlas_texture, GL_TEXTURE3, icon_atlas);
}

Backend_OpenGL::~Backend_OpenGL()
{
    // destroy vertex and index buffers
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &index_buffer);

    // destroy shader
    glDeleteProgram(shader_program);

    // destroy textures
    glDeleteTextures(1, &text_atlas_texture);
    glDeleteTextures(1, &text_bold_atlas_texture);
    glDeleteTextures(1, &slice_atlas_texture);
    glDeleteTextures(1, &icon_atlas_texture);
}

void Backend_OpenGL::mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
{
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()),
        vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned int) * indices.size()),
        indices.data(), GL_STATIC_DRAW);

    index_count = static_cast<int>(indices.size());
}

void Backend_OpenGL::bind(std::shared_ptr<Window> window) const
{
    glBindVertexArray(vertex_array_object);

    glUseProgram(shader_program);

    glm::vec2 size = window->getSize();
    auto transform = glm::ortho(0.0f, size.x, size.y, 0.0f, 1000.0f, -1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "world_to_clip"), 1, GL_FALSE,
        (float*)&transform);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, text_atlas_texture);
    glUniform1i(glGetUniformLocation(shader_program, "text_atlas"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, text_bold_atlas_texture);
    glUniform1i(glGetUniformLocation(shader_program, "text_bold_atlas"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, slice_atlas_texture);
    glUniform1i(glGetUniformLocation(shader_program, "slice_atlas"), 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, icon_atlas_texture);
    glUniform1i(glGetUniformLocation(shader_program, "icon_atlas"), 3);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Backend_OpenGL::draw(std::shared_ptr<Window> window) const
{ glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr); }
