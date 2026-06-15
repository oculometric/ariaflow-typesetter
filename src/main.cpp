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

    UIRootMenu* root_menu = new UIRootMenu();
    root_menu->addLabel("", 12);

    UIMenu* file_menu = root_menu->addSubMenu("file");
    file_menu->addButton("new", []() -> void { std::cout << "new" << std::endl; }, "Ctrl+N");
    file_menu->addButton("open...", []() -> void { std::cout << "open" << std::endl; }, "Ctrl+O", 14);
    UIMenu* recents = file_menu->addSubMenu("open recent");
    recents->addButton("item 1", []() -> void { std::cout << "test" << std::endl; });
    recents->addButton("item 2", nullptr);
    recents->addButton("item 3", nullptr);
    recents->addButton("item 4", nullptr);
    file_menu->addButton("export...", nullptr, "Ctrl+E", 13);
    file_menu->addButton("save", nullptr, "Ctrl+S", 11);
    file_menu->addButton("save as...", nullptr, "", 11);
    file_menu->addButton("save incremental", nullptr, "Ctrl+Alt+I", 11);
    file_menu->addButton("revert", nullptr, "", 15);
    file_menu->addDivider();
    file_menu->addButton("exit", []() -> void { exit(1); }, "Alt+F4");

    UIMenu* edit_menu = root_menu->addSubMenu("edit");
    edit_menu->addButton("copy", nullptr, "Ctrl+C");
    edit_menu->addButton("cut", nullptr, "Ctrl+X");
    edit_menu->addButton("paste", nullptr, "Ctrl+V");
    edit_menu->addDivider();
    edit_menu->addButton("undo", nullptr, "Ctrl+Z");
    edit_menu->addButton("redo", nullptr, "Ctrl+Shft+Z");
    edit_menu->addDivider();
    edit_menu->addButton("select all", nullptr, "Ctrl+A");
    edit_menu->addButton("select paragraph", nullptr, "Ctrl+Shft+A");
    edit_menu->addDivider();
    edit_menu->addButton("settings", nullptr, "Ctrl+,");

    UIMenu* scripts_menu = root_menu->addSubMenu("scripts");
    scripts_menu->addLabel("you have no scripts.", 7);

    UIMenu* view_menu = root_menu->addSubMenu("view");
    view_menu->addButton("show raw view", nullptr, "Alt+R");
    view_menu->addButton("show metrics", nullptr, "Alt+M");
    view_menu->addButton("show guides", nullptr, "Alt+G", 10);
    view_menu->addDivider();
    view_menu->addButton("reset layout", nullptr, "", 15);

    UIMenu* help_menu = root_menu->addSubMenu("help");
    help_menu->addButton("about", nullptr);
    help_menu->addDivider();
    help_menu->addButton(
        "repository",
        []() -> void
        {
#if defined(_WIN32)
            system("start https://github.com/oculometric/typesetter-project");
#else
            system("xdg-open https://github.com/oculometric/typesetter-project");
#endif
        },
        "", 13);

    root_menu->addButton("test", []() -> void { std::cout << "test" << std::endl; });

    // TODO: make 'buttonpanel' a thing
    std::vector<UIButton*> buttons;
    buttons.push_back(new UIButton("", []() { std::cout << "hi" << std::endl; }, 0));
    buttons.push_back(new UIButton("", nullptr, 1));
    buttons.push_back(new UIButton("", nullptr, 2));
    buttons.push_back(new UIButton("", nullptr, 3));
    buttons.push_back(new UIButton("", nullptr, 4));
    buttons.push_back(new UIButton("", nullptr, 5));
    buttons.push_back(new UIButton("", nullptr, 6));
    buttons.push_back(new UIButton("", nullptr, 7));
    buttons.push_back(new UIButton("", nullptr, 8));
    buttons.push_back(new UIButton("", nullptr, 9));
    buttons.push_back(new UIButton("", nullptr, 10));
    buttons.push_back(new UIButton("", nullptr, 11));
    buttons.push_back(new UIButton("", nullptr, 12));
    buttons.push_back(new UIButton("", nullptr, 13));
    buttons.push_back(new UIButton("", nullptr, 14));
    buttons.push_back(new UIButton("", nullptr, 15));
    UIPanel* button_panel          = new UIPanel({ 0.4f, 0.4f, 0.4f, 1.0f }, 1, 0b1111);
    int palette_columns            = 2;
    const glm::vec2 button_size    = buttons[0]->getSize(r);
    glm::vec2 palette_size         = { button_size.x * palette_columns,
        (button_size.y *
            glm::ceil(static_cast<float>(buttons.size()) / static_cast<float>(palette_columns))) };
    glm::vec2 palette_top_left     = { w->getSize().x - (palette_size.x + 16), root_menu->getHeight() + 8 };
    UIGrabbable* palette_grabbale  = new UIGrabbable(CURSOR_HAND);
    UIGrabbable* palette_left_grab = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);
    UIGrabbable* palette_right_grab = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);

    UITextEditor* raw_editor      = new UITextEditor();
    glm::vec2 raw_editor_top_left = { w->getSize().x / 2.0f, root_menu->getHeight() };
    glm::vec2 raw_editor_size     = { w->getSize().x / 2.0f, w->getSize().y - raw_editor_top_left.y };

    while (!w->shouldClose())
    {
        glm::vec2 old_window_size = w->getSize();

        // render
        w->makeCurrentContext();
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        r->draw(w);
        w->present();
        w->poll();
        w->setCursorType(CURSOR_NORMAL);

        glm::vec2 new_window_size = w->getSize();

        // build ui and check input
        r->clear();

        root_menu->checkInput(w);
        root_menu->draw(r, w->getSize().x);

        {
            glm::vec2 palette_midpoint = palette_top_left + (palette_size / 2.0f);

            if (palette_midpoint.x > old_window_size.x / 2.0f)
                palette_top_left.x += (new_window_size - old_window_size).x;

            if (glm::abs(palette_midpoint.y - (old_window_size.y / 2.0f)) < (old_window_size.y / 8.0f))
                palette_top_left.y += (new_window_size - old_window_size).y / 2.0f;

            else if (palette_midpoint.y > old_window_size.y / 2.0f)
                palette_top_left.y += (new_window_size - old_window_size).y;

            const glm::vec2 button_size = buttons[0]->getSize(r);

            palette_top_left = palette_grabbale->checkInput(w, palette_top_left, { palette_size.x, 12 });

            // TODO: fix overlapping left/right grab and top grab
            glm::vec2 palette_left =
                palette_left_grab->checkInput(w, palette_top_left, { 4, palette_size.y });
            float size_change = palette_left.x - palette_top_left.x;
            palette_top_left.x += size_change;
            palette_size.x -= size_change;
            glm::vec2 palette_right = palette_right_grab->checkInput(w,
                palette_top_left + glm::vec2{ palette_size.x + 4, 0 }, { 4, palette_size.y });
            size_change             = palette_right.x - (palette_top_left.x + palette_size.x + 4);
            palette_size.x += size_change;
            palette_columns =
                glm::min(glm::max(1, static_cast<int>(glm::floor(palette_size.x / button_size.x))),
                    static_cast<int>(buttons.size()));
            palette_size.y = (button_size.y * glm::ceil(static_cast<float>(buttons.size()) /
                                                        static_cast<float>(palette_columns)));
            if (!palette_left_grab->isCurrentlyGrabbed() && !palette_right_grab->isCurrentlyGrabbed())
            {
                palette_size.x = button_size.x * palette_columns;
            }

            palette_top_left = glm::clamp(palette_top_left, { 0, root_menu->getHeight() },
                new_window_size - glm::vec2{ palette_size.x + 8, palette_size.y + 12 + 4 });

            glm::vec2 temp = glm::round(palette_top_left);
            button_panel->draw(r, temp,
                glm::round(glm::vec2{ palette_size.x + 8, palette_size.y + 12 + 4 }));
            temp.y += 12;
            temp.x += 4;

            int col      = 0;
            float height = 0;
            for (auto button : buttons)
            {
                button->checkInput(w, temp + glm::vec2{ button_size.x * col, height });
                button->draw(r, temp + glm::vec2{ button_size.x * col, height });
                if ((col + 1) % palette_columns == 0) height += button_size.y;
                col = (col + 1) % palette_columns;
            }
        }

        raw_editor->checkInput(w, raw_editor_top_left, raw_editor_size);
        raw_editor->draw(r, raw_editor_top_left, raw_editor_size);

        consumeAllMouseEvents(w);
        r->finalise();
    }

    delete w;

    return 0;
}