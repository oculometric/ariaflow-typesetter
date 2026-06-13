#include "user_interface.h"
#include "window.h"

#include <glad.h>
#include <iostream>

using namespace AriaFlow;

int main()
{
    Window* w = new Window();
    w->makeCurrentContext();
    UIRenderer* r = new UIRenderer();

    const float line_height = 24.0f;
    const float icon_size = 24.0f;
    const float spacing = 2.0f;
    const float text_push = 3.0f;
    const glm::vec4 panel_colour = { 0.12f, 0.12f, 0.12f, 1.0f };
    const glm::vec4 panel_sec_colour = { 0.3f, 0.3f, 0.3f, 1.0f };
    const glm::vec3 text_colour = { 0.9f, 0.9f, 0.9f };
    const glm::vec3 text_sec_colour = { 0.5f, 0.5f, 0.5f };

    // r->addText({ 0, 0 }, 0.0f, {}, "Hello, World!", { 1, 1, 1 });
    // r->addNineSlice({ 36 * 0, 64 }, 0.0f, { 36, 36 }, 0, { 1, 1, 1, 1 });
    // r->addNineSlice({ 36 * 1, 64 }, 0.0f, { 36, 36 }, 1, { 1, 1, 1, 1 });
    // r->addNineSlice({ 36 * 2, 64 }, 0.0f, { 36, 36 }, 2, { 1, 1, 1, 1 });
    // r->addNineSlice({ 36 * 2, 64 }, 0.0f, { 36, 36 }, 3, { 1, 1, 1, 1 });
    // float x = 0;
    // for (int i = 0; i < 16; ++i, x += 12)
    // {
    //     r->addSimple({ x, 128 }, 0.0f, { 12, 12 }, i, { 0, 0 }, { 1, 1 });
    // }

    while (!w->shouldClose())
    {
        // TODO: process input
        // TODO: render UI

        {
            glm::vec2 mouse = w->getMousePosition();
            r->clear();
            float file_menu_x;
            float edit_menu_x;
            float scripts_menu_x;
            float view_menu_x;
            float help_menu_x;
            float end_menu_x;
            float menu_bottom = line_height + (spacing * 2);
            { // main menu
                r->addNineSlice({ 0, 0 }, -2, { 1024, menu_bottom }, 0, panel_colour, 0b0010);
                float offset = spacing;
                r->addSimple({ offset, spacing }, 0, { icon_size, icon_size }, 12, { 0, 0 }, { 1, 1 });
                offset += icon_size + (spacing * 2);
                file_menu_x = offset;
                offset += r->addText({ offset, text_push + spacing }, 0, {}, "file ", text_colour).x + (spacing * 2);
                edit_menu_x = offset;
                offset += r->addText({ offset, text_push + spacing }, 0, {}, "edit ", text_colour).x + (spacing * 2);
                scripts_menu_x = offset;
                offset += r->addText({ offset, text_push + spacing }, 0, {}, "scripts ", text_colour).x + (spacing * 2);
                view_menu_x = offset;
                offset += r->addText({ offset, text_push + spacing }, 0, {}, "view ", text_colour).x + (spacing * 2);
                help_menu_x = offset;
                offset += r->addText({ offset, text_push + spacing }, 0, {}, "help ", text_colour).x + (spacing * 2);
                end_menu_x = offset;
            }

            if (mouse.y <= menu_bottom && mouse.x >= file_menu_x && mouse.x < edit_menu_x)
            { // file menu
                float left = file_menu_x - spacing;
                r->addNineSlice({ left, 0 }, -1, { edit_menu_x - file_menu_x, menu_bottom }, 3, panel_sec_colour, 0b0010);
                float right = left + 324 + (spacing * 2);
                float top = menu_bottom;
                r->addNineSlice({ left, top }, 0, { right - left, (line_height * 8) + (spacing * 2) }, 0, panel_colour, 0b1111);
                left += spacing + spacing;
                right -= spacing + spacing;
                top += text_push;
                r->addText({ left, top }, 0, {}, "new", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+N", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "open...", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+O", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "export...", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+E", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "save", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+S", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "save as...", text_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "save incremental", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+Alt+I", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "revert", text_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "exit", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Alt+F4", text_sec_colour);
                top += line_height;
            }

            if (mouse.y <= menu_bottom && mouse.x >= edit_menu_x && mouse.x < scripts_menu_x)
            { // edit menu
                float left = edit_menu_x - spacing;
                r->addNineSlice({ left, 0 }, -1, { scripts_menu_x - edit_menu_x, menu_bottom }, 3, panel_sec_colour, 0b0010);
                float right = left + 332 + (spacing * 2);
                float top = menu_bottom;
                r->addNineSlice({ left, top }, 0, { right - left, (line_height * 8) + (spacing * 2) }, 0, panel_colour, 0b1111);
                left += spacing + spacing;
                right -= spacing + spacing;
                top += text_push;
                r->addText({ left, top }, 0, {}, "copy", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+C", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "cut", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+X", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "paste", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+V", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "undo", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+Z", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "redo", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+Shft+Z", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "select all", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+A", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "select paragraph", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+Shft+A", text_sec_colour);
                top += line_height;
                r->addText({ left, top }, 0, {}, "settings", text_colour);
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Ctrl+,", text_sec_colour);
                top += line_height;
            }

            if (mouse.y <= menu_bottom && mouse.x >= view_menu_x && mouse.x < help_menu_x)
            { // view menu
                float left = view_menu_x - spacing;
                r->addNineSlice({ left, 0 }, -1, { help_menu_x - view_menu_x, menu_bottom }, 3, panel_sec_colour, 0b0010);
                float right = left + 208 + (spacing * 2);
                float top = line_height + (spacing * 2);
                r->addNineSlice({ left, top }, 0, { right - left, (line_height * 3) + (spacing * 2) }, 0, panel_colour, 0b1111);
                left += spacing + spacing;
                right -= spacing + spacing;
                top += text_push;
                r->addText({ left, top }, 0, {}, "reset layout", text_colour);
                top += line_height;
                float off = r->addText({ left, top }, 0, {}, "raw view", text_colour).x;
                r->addSimple({ left + off + (spacing * 3), top }, 0, { icon_size, icon_size }, 9, { 0, 0 }, { 1, 1 });
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Alt+R", text_sec_colour);
                top += line_height;
                off = r->addText({ left, top }, 0, {}, "metrics", text_colour).x;
                r->addSimple({ left + off + (spacing * 3), top }, 0, { icon_size, icon_size }, 0, { 0, 0 }, { 1, 1 });
                r->addText({ right, top }, 0, { TEXT_ALIGN_RIGHT }, "Alt+M", text_sec_colour);
                top += line_height;
            }

            if (mouse.y <= menu_bottom && mouse.x >= help_menu_x && mouse.x < end_menu_x)
            { // help menu
                float left = help_menu_x - spacing;
                r->addNineSlice({ left, 0 }, -1, { end_menu_x - help_menu_x, menu_bottom }, 3, panel_sec_colour, 0b0010);
                float right = left + 164 + (spacing * 2);
                float top = line_height + (spacing * 2);
                r->addNineSlice({ left, top }, 0, { right - left, (line_height * 2) + (spacing * 2) }, 0, panel_colour, 0b1111);
                left += spacing + spacing;
                right -= spacing + spacing;
                top += text_push;
                r->addText({ left, top }, 0, {}, "about", text_colour);
                top += line_height;
                float off = r->addText({ left, top }, 0, {}, "repository", text_colour).x;
                r->addSimple({ left + off + (spacing * 3), top }, 0, { icon_size, icon_size }, 13, { 0, 0 }, { 1, 1 });
                top += line_height;
            }
        }
        r->finalise();


        w->makeCurrentContext();
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        r->draw(w);
        w->present();
        w->poll();
    }

    delete w;

    return 0;
}