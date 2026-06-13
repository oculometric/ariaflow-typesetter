namespace AriaFlow
{

static const char* vertex_shader_source = R"(
#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_colour_1;
layout (location = 2) in vec4 vertex_colour_2;
layout (location = 3) in vec4 vertex_data_1;
layout (location = 4) in vec4 vertex_data_2;
layout (location = 5) in vec2 vertex_uv;

varying vec2 position;
varying vec4 colour_1;
varying vec4 colour_2;
varying vec4 data_1;
varying vec4 data_2;
varying vec2 uv;

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

varying vec2 position;
varying vec4 colour_1;
varying vec4 colour_2;
varying vec4 data_1;
varying vec4 data_2;
varying vec2 uv;

out vec4 frag_colour;

uniform sampler2D text_atlas; // 16x16
uniform sampler2D text_bold_atlas; // 16x16
uniform sampler2D slice_atlas; // 2x2
uniform sampler2D icon_atlas; // 8x8
uniform sampler2D line_atlas;

void main()
{
    int draw_mode = int(round(data_1.x));
    vec2 quad_size = data_1.yz;
    ivec2 screen_coord = ivec2(position.xy);

    if (draw_mode == 0)         // text mode
    {
        uint segment = uint(round(data_1.w));
        uint mode = uint(round(data_2.x));
        bool bold = bool((text_mode & 1));
        bool underline = bool(text_mode & 2);
        bool strikethrough = bool(text_mode & 4);

        vec2 te_uv = uv;
        vec2 te_off = vec2(float(segment % 16), float((segment / 16) % 16));
        vec2 te_size = quad_size / vec2(textureSize(text_atlas, 0.0f));

        float tex_value = 0.0f;
        if (bold)
            tex_value = texture(text_bold_atlas, (te_uv + te_off) / 16.0f).r;
        else
            tex_value = texture(text_atlas, (te_uv + te_off) / 16.0f).r;
        
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

        vec4 target_colour = (tex_value > 0.5f) ? colour_1 : colour_2;
        if (target_colour.a < 0.5f)
            discard;
        frag_colour = vec4(target_colour.rgb, 1.0f);
    }
    else if (draw_mode == 1)    // 9-slice mode
    {
        vec2 atlas_size = vec2(textureSize(slice_atlas, 0.0f)) / 2.0f;
        uint segment = uint(round(data_1.w));
        uint borders = uint(round(data_2.x));

        vec2 ns_uv = nineSliceUV(uv.xy, quad_size, atlas_size, bool(borders & 1), bool(borders & 2), bool(borders & 4), bool(borders & 8));
        vec2 ns_off = vec2(float(segment % 2), float((segment / 2) % 2));
        
        vec4 tex_colour = texture(slice_atlas, (ns_uv + ns_off) / 2.0f);

        vec4 target_colour = (tex_colour.rgb == vec3(1.0f, 0.0f, 1.0f)) ? colour_1 : (tex_colour.rgb * colour_2);
        if (target_colour.a < 0.5f)
            discard;
        frag_colour = vec4(target_colour.rgb, 1.0f);
    }
    else if (draw_mode == 2)    // icon mode
    {
        vec2 atlas_size = vec2(textureSize(icon_atlas, 0.0f)) / 8.0f;
        uint segment = uint(round(data_1.w));

        vec2 ic_uv = uv;
        vec2 ic_off = vec2(float(segment % 8), float((segment / 8) % 8));

        vec4 tex_colour = texture(icon_atlas, (ic_uv + ic_off) / 8.0f);

        vec4 target_colour = (tex_colour.a > 0.5f) ? (tex_colour.rgb * colour_1) : colour_2;
        if (target_colour.a < 0.5f)
            discard;
        frag_colour = vec4(target_colour.rgb, 1.0f);
    }
    else if (draw_mode == 3)    // line mode
    {
        // TODO: line mode
    }
}
)";

}; // namespace AriaFlow