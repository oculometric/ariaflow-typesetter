#include "shaders.h"
#include "textures.h"
#include "user_interface.h"
#include "window.h"

#include <cstring>
#include <glad.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <stdexcept>

using namespace AriaFlow;

UIRenderer::UIRenderer(int font_index)
{
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &index_buffer);
    next_id = 123;

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
    transform_var = glGetUniformLocation(shader_program, "world_to_clip");
    glUniform1i(glGetUniformLocation(shader_program, "text_atlas"), 0);
    glUniform1i(glGetUniformLocation(shader_program, "text_bold_atlas"), 1);
    glUniform1i(glGetUniformLocation(shader_program, "slice_atlas"), 2);
    glUniform1i(glGetUniformLocation(shader_program, "icon_atlas"), 3);
    glUniform1i(glGetUniformLocation(shader_program, "line_atlas"), 4);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
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

    auto make_tex = [](unsigned int* texture, int slot, unsigned char* data, size_t data_size) -> void
    {
        glGenTextures(1, texture);
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        int x, y, c;
        stbi_uc* pixels = stbi_load_from_memory(data, data_size, &x, &y, &c, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        stbi_image_free(pixels);
    };

    if (font_index == 1)
    {
        make_tex(&text_atlas_texture, GL_TEXTURE0, ibm_font, ibm_font_size);
        make_tex(&text_bold_atlas_texture, GL_TEXTURE1, ibm_font, ibm_font_size);
        text_size = { 12, 23 };
    }
    else
    {
        make_tex(&text_atlas_texture, GL_TEXTURE0, roboto_font, roboto_font_size);
        make_tex(&text_bold_atlas_texture, GL_TEXTURE1, roboto_font_bold, roboto_font_bold_size);
        text_size = { 34, 62 };
    }
    make_tex(&slice_atlas_texture, GL_TEXTURE2, slices, slices_size);
    make_tex(&icon_atlas_texture, GL_TEXTURE3, icons, icons_size);
    make_tex(&line_atlas_texture, GL_TEXTURE4, icons, icons_size);
}

UIRenderer::~UIRenderer() {}

void UIRenderer::clear()
{
    transform = glm::mat3(1.0);
    vertices.clear();
    indices.clear();
    backing_datas.clear();
}

void UIRenderer::addQuad(float z, BackingData& backing_ref)
{
    if (isBackingValid(backing_ref)) return;

    if (!indices.count(z)) indices[z] = {};

    auto& _indices = indices[z];

    BackingDataInternal backing;
    backing.first_vertex = static_cast<uint16_t>(vertices.size());
    backing.vertex_count = 4;
    backing.first_index  = static_cast<uint16_t>(_indices.size());
    backing.index_count  = 6;
    backing.z            = z;

    vertices.push_back({});
    vertices.push_back({});
    vertices.push_back({});
    vertices.push_back({});

    _indices.push_back(0);
    _indices.push_back(0);
    _indices.push_back(0);
    _indices.push_back(0);
    _indices.push_back(0);
    _indices.push_back(0);

    addBacking(backing_ref, backing);
}

void UIRenderer::addQuad(float z)
{
    BackingData backing;
    addQuad(z, backing);
}

void UIRenderer::addQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float z, glm::vec2 uv_tl,
    glm::vec2 uv_br, glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2,
    BackingData& backing_ref)
{
    addQuad(z, backing_ref);
    BackingDataInternal backing = backing_datas[backing_ref.id];

    updateQuad(p1, p2, p3, p4, uv_tl, uv_br, colour_1, colour_2, data_1, data_2, backing);
}

void UIRenderer::addQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float z, glm::vec2 uv_tl,
    glm::vec2 uv_br, glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2)
{
    BackingData backing;
    addQuad(p1, p2, p3, p4, z, uv_tl, uv_br, colour_1, colour_2, data_1, data_2, backing);
}

float UIRenderer::calculateTextWidth(const std::string& text, TextFormatting formatting)
{
    float ratio = formatting.size / text_size.y;
    return static_cast<float>(
        (static_cast<float>(text.size()) * ((text_size.x * ratio) + formatting.spacing)) -
        formatting.spacing);
}

glm::vec2 UIRenderer::addText(glm::vec2 position, float z, TextFormatting formatting,
    const std::string& text, glm::vec3 colour, BackingData& backing_ref)
{
    BackingDataInternal backing;
    if (!isBackingValid(backing_ref))
    {
        if (!indices.count(z)) indices[z] = {};

        auto& _indices = indices[z];

        backing.first_vertex = static_cast<uint16_t>(vertices.size());
        backing.vertex_count = static_cast<uint16_t>(4 * text.size());
        backing.first_index  = static_cast<uint16_t>(_indices.size());
        backing.index_count  = static_cast<uint16_t>(6 * text.size());
        backing.z            = z;

        for (size_t i = 0; i < text.size(); ++i)
        {
            vertices.push_back({});
            vertices.push_back({});
            vertices.push_back({});
            vertices.push_back({});

            _indices.push_back(0);
            _indices.push_back(0);
            _indices.push_back(0);
            _indices.push_back(0);
            _indices.push_back(0);
            _indices.push_back(0);
        }

        addBacking(backing_ref, backing);
    }
    else
    {
        backing = backing_datas[backing_ref.id];
    }

    const glm::vec2 char_size = text_size * (formatting.size / text_size.y);
    size_t allocated_chars    = backing.vertex_count / 4;
    const size_t chars_wide =
        static_cast<size_t>(glm::floor(static_cast<float>(formatting.clip_bounds.x) / char_size.x));
    std::vector<std::string> lines;

    if (!formatting.wrap || (formatting.wrap && !formatting.clip))
    {
        if (formatting.terminate_at_newline) lines.push_back(text.substr(0, text.find('\n')));
        else
        {
            size_t newline = 0;
            size_t next    = 0;
            do
            {
                next             = text.find('\n', newline);
                std::string line = text.substr(newline, (next - newline));
                if (formatting.clip)
                {
                    while (calculateTextWidth(line.substr(0, line.size() - 1), formatting) >
                           formatting.clip_bounds.x)
                        line.pop_back();
                }
                lines.push_back(line);
                newline = next + 1;
            } while (next != std::string::npos);
        }
    }
    else
    {
        size_t base = 0;
        while (base < text.size())
        {
            size_t split    = std::min(base + chars_wide, text.size());
            size_t new_base = split;
            size_t newline  = text.find('\n', base);
            if (newline <= split)
            {
                split = newline;
                if (newline == split) new_base = split + 1;
                else
                    new_base = split;
            }
            else
            {
                while (split >= base)
                {
                    if (split < text.size() && (text[split] == ' ' || text[split] == '\n'))
                    {
                        new_base = split + 1;
                        break;
                    }
                    --split;
                    if (split == base)
                    {
                        split    = base + chars_wide;
                        new_base = split;
                        break;
                    }
                }
            }

            lines.push_back(text.substr(base, split - base));
            base = new_base;
            if (split < text.size() && text[split] == '\n' && formatting.terminate_at_newline) break;
        }
    }

    BackingDataInternal temp = backing;
    temp.vertex_count        = 4;
    temp.index_count         = 6;

    glm::vec2 top_left = position;
    int bottom_clip    = formatting.clip_bounds.y;
    if (bottom_clip <= 0 || !formatting.clip) bottom_clip = INT_MAX;
    for (const auto& line : lines)
    {
        TextFormatting sub_format = formatting;
        sub_format.clip_bounds.y  = bottom_clip;
        if (allocated_chars < line.size())
        {
            updateTextSingleLine(top_left, sub_format, line.substr(0, allocated_chars), colour, temp);
            break;
        }
        updateTextSingleLine(top_left, sub_format, line, colour, temp);
        allocated_chars -= line.size();

        top_left.y += char_size.y;
        bottom_clip -= static_cast<int>(char_size.y);
        temp.first_vertex += 4 * static_cast<uint16_t>(line.size());
        temp.first_index += 6 * static_cast<uint16_t>(line.size());
    }

    float longest = 0;
    for (const auto& line : lines)
    {
        float length = calculateTextWidth(line, formatting);
        if (length > longest) longest = length;
    }

    return glm::vec2{ longest, char_size.y * static_cast<float>(lines.size()) };
}

glm::vec2 UIRenderer::addText(glm::vec2 position, float z, TextFormatting formatting,
    const std::string& text, glm::vec3 colour)
{
    BackingData backing;
    return addText(position, z, formatting, text, colour, backing);
}

void UIRenderer::addNineSlice(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec4 fill,
    uint8_t borders, BackingData& backing_ref)
{
    addQuad(position, position + glm::vec2{ size.x, 0 }, position + glm::vec2{ 0, size.y }, position + size,
        z, { 0, 0 }, { 1, 1 }, fill, { 1, 1, 1, 1 }, glm::vec4{ 1, size, layer },
        glm::vec4{ static_cast<float>(borders), 0, 0, 0 }, backing_ref);
}

void UIRenderer::addNineSlice(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec4 fill,
    uint8_t borders)
{
    BackingData backing;
    addNineSlice(position, z, size, layer, fill, borders, backing);
}

void UIRenderer::addSimple(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec2 uv_base,
    glm::vec2 uv_size, BackingData& backing_ref)
{
    addQuad(position, position + glm::vec2{ size.x, 0 }, position + glm::vec2{ 0, size.y }, position + size,
        z, uv_base, uv_base + uv_size, glm::vec4{ 1, 1, 1, 1 }, glm::vec4{ 1, 1, 1, 0 },
        glm::vec4{ 2, size, layer }, glm::vec4{ 0, 0, 0, 0 }, backing_ref);
}

void UIRenderer::addSimple(glm::vec2 position, float z, glm::vec2 size, int layer, glm::vec2 uv_base,
    glm::vec2 uv_size)
{
    BackingData backing;
    addSimple(position, z, size, layer, uv_base, uv_size, backing);
}

void UIRenderer::finalise()
{
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()),
        vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    std::vector<unsigned int> final_indices;
    size_t total_indices = 0;
    for (const auto& arr : indices) total_indices += arr.second.size();
    final_indices.resize(total_indices);
    size_t offset = 0;

    for (const auto& arr : indices)
    {
        memcpy(final_indices.data() + offset, arr.second.data(), arr.second.size() * sizeof(unsigned int));
        offset += arr.second.size();
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(sizeof(unsigned int) * final_indices.size()), final_indices.data(),
        GL_STATIC_DRAW);
    index_count = static_cast<int>(final_indices.size());
}

void UIRenderer::draw(Window* window) const
{
    window->makeCurrentContext();

    static int scale_factor = 1;
    int width               = window->getSize().x;
    int height              = window->getSize().y;
    glm::vec2 size          = window->getSize();

    auto transform = glm::ortho(0.0f, size.x, size.y, 0.0f, 1000.0f, -1000.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, text_atlas_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, text_bold_atlas_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, slice_atlas_texture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, icon_atlas_texture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, line_atlas_texture);
    glBindVertexArray(vertex_array_object);
    glUniformMatrix4fv(transform_var, 1, GL_FALSE, (float*)&transform);
    glUniform1i(glGetUniformLocation(shader_program, "text_atlas"), 0);
    glUniform1i(glGetUniformLocation(shader_program, "text_bold_atlas"), 1);
    glUniform1i(glGetUniformLocation(shader_program, "slice_atlas"), 2);
    glUniform1i(glGetUniformLocation(shader_program, "icon_atlas"), 3);
    glUniform1i(glGetUniformLocation(shader_program, "line_atlas"), 4);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(shader_program);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
}

bool UIRenderer::isBackingValid(const BackingData& backing_ref)
{ return backing_datas.count(backing_ref.id); }

void UIRenderer::addBacking(BackingData& backing_ref, BackingDataInternal backing)
{
    backing_datas[next_id] = backing;
    backing_ref.id         = next_id;
    ++next_id;
    if (next_id == 0) next_id = 4;
}

void UIRenderer::updateTextSingleLine(glm::vec2 position, TextFormatting formatting,
    const std::string& text, glm::vec3 colour, BackingDataInternal backing)
{
    const glm::vec2 uv_size   = text_size / (text_size + 2.0f);
    const glm::vec2 char_size = text_size * (formatting.size / text_size.y);
    const float width         = calculateTextWidth(text, formatting);

    glm::vec2 top_left = position;
    if (formatting.align == TEXT_ALIGN_RIGHT) top_left.x -= width;
    else if (formatting.align == TEXT_ALIGN_CENTER)
        top_left.x -= glm::round(width / 2.0f);

    BackingDataInternal temp = backing;
    temp.vertex_count        = 4;
    temp.index_count         = 6;

    for (char c : text)
    {
        glm::vec2 uv_base = 1.0f / (text_size + 2.0f);

        glm::vec2 uv_br = uv_base + uv_size;
        glm::vec2 uv_tl = uv_base;

        glm::vec2 skew = { 0, 0 };
        if (formatting.flags & TEXT_FLAGS_ITALIC) skew.x = glm::round(char_size.x / 2.0f);

        int flags = 0;
        if (formatting.flags & TEXT_FLAGS_BOLD) flags |= TEXT_FLAGS_BOLD;
        if (formatting.flags & TEXT_FLAGS_UNDERLINE) flags |= TEXT_FLAGS_UNDERLINE;
        if (formatting.flags & TEXT_FLAGS_STRIKETHROUGH) flags |= TEXT_FLAGS_STRIKETHROUGH;

        glm::vec2 tl = top_left + skew;
        glm::vec2 tr = top_left + glm::vec2{ char_size.x, 0 } + skew;
        glm::vec2 bl = top_left + glm::vec2{ 0, char_size.y };
        glm::vec2 br = top_left + char_size;

        if (char_size.y > formatting.clip_bounds.y)
        {
            float subtract_amount_px = glm::min(char_size.y, char_size.y - formatting.clip_bounds.y);
            float subtract_amount_uv = (uv_size.y / char_size.y) * subtract_amount_px;
            bl.y -= subtract_amount_px;
            br.y -= subtract_amount_px;
            uv_br.y += subtract_amount_uv;
        }

        if (formatting.clip && tr.x > formatting.clip_bounds.x)
        {
            float subtract_amount_px = glm::min(tr.x, tr.x - formatting.clip_bounds.x);
            float subtract_amount_uv = (uv_size.x / char_size.x) * subtract_amount_px;
            tr.x -= subtract_amount_px;
            br.x -= subtract_amount_px;
            uv_br.x -= subtract_amount_uv;
        }

        updateQuad(tl, tr, bl, br, uv_tl, uv_br, glm::vec4{ colour, 1 }, background_colour,
            glm::vec4{ 0, char_size, static_cast<float>(c) },
            glm::vec4{ static_cast<float>(flags), 0, 0, 0 }, temp);

        top_left.x += char_size.x + formatting.spacing;

        temp.first_vertex += 4;
        temp.first_index += 6;
    }
}

void UIRenderer::updateQuad(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 uv_tl,
    glm::vec2 uv_br, glm::vec4 colour_1, glm::vec4 colour_2, glm::vec4 data_1, glm::vec4 data_2,
    BackingDataInternal backing)
{
    glm::vec3 _p1 = transform * glm::vec3{ p1, 1 };
    glm::vec3 _p2 = transform * glm::vec3{ p2, 1 };
    glm::vec3 _p3 = transform * glm::vec3{ p3, 1 };
    glm::vec3 _p4 = transform * glm::vec3{ p4, 1 };

    // top left
    vertices[backing.first_vertex + 0] = Vertex{
        { _p1.x, _p1.y, backing.z },
        colour_1, colour_2, data_1, data_2, uv_tl
    };
    // top right
    vertices[backing.first_vertex + 1] = Vertex{
        { _p2.x, _p2.y, backing.z },
        colour_1, colour_2, data_1, data_2, { uv_br.x, uv_tl.y }
    };
    // bottom left
    vertices[backing.first_vertex + 2] = Vertex{
        { _p3.x, _p3.y, backing.z },
        colour_1, colour_2, data_1, data_2, { uv_tl.x, uv_br.y }
    };
    // bottom right
    vertices[backing.first_vertex + 3] = Vertex{
        { _p4.x, _p4.y, backing.z },
        colour_1, colour_2, data_1, data_2, uv_br
    };

    auto& _indices = indices[backing.z];

    _indices[backing.first_index + 0] = (backing.first_vertex + 0);
    _indices[backing.first_index + 1] = (backing.first_vertex + 3);
    _indices[backing.first_index + 2] = (backing.first_vertex + 1);
    _indices[backing.first_index + 3] = (backing.first_vertex + 0);
    _indices[backing.first_index + 4] = (backing.first_vertex + 2);
    _indices[backing.first_index + 5] = (backing.first_vertex + 3);
}
