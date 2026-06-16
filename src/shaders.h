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
    else if (draw_mode == 3)    // line mode
    {
        // TODO: line mode
    }
}
)";

}; // namespace AriaFlow