#include "document.h"
#include "user_interface.h"
#include "window.h"

#include <glad.h>
#include <iostream>

using namespace AriaFlow;

int main()
{
    Window* w = new Window();
    w->makeCurrentContext();
    UIRenderer* r  = new UIRenderer();
    UIRenderer* r2 = new UIRenderer(1);

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

    Document* d = new Document();

    UIResizablePanel* raw_editor =
        new UIResizablePanel({ 64, 128 }, { w->getSize().x / 2.0f, UIRootMenu::getHeight() },
            { w->getSize().x / 2.0f, w->getSize().y - UIRootMenu::getHeight() });
    raw_editor->title         = "raw view";
    UITextEditor* text_editor = new UITextEditor();
    text_editor->custom_r     = r2;
    text_editor->data_source  = d;
    raw_editor->child         = text_editor;

    UIResizablePanel* preview_editor = new UIResizablePanel({ 64, 128 }, { 0, UIRootMenu::getHeight() },
        { w->getSize().x / 2.0f, w->getSize().y - UIRootMenu::getHeight() });
    preview_editor->title            = "rendered view";

    UIRootMenu* root_menu = new UIRootMenu();
    root_menu->addLabel("", 12);

    UIMenu* file_menu = root_menu->addSubMenu("file");
    // TODO: check for unsaved changes!
    file_menu->addButton(
        "new",
        [&]() -> void
        {
            d                        = new Document();
            text_editor->data_source = d;
        },
        "Ctrl+N");
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
    file_menu->addButton("exit", []() -> void { exit(EXIT_FAILURE); }, "Alt+F4");

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
    UIMenu* view_layouts = view_menu->addSubMenu("reset layout", 15);
    view_layouts->addButton("side-by-side",
        [&]()
        {
            raw_editor->position = { w->getSize().x / 2.0f, UIRootMenu::getHeight() };
            raw_editor->size     = { w->getSize().x / 2.0f, w->getSize().y - UIRootMenu::getHeight() };

            preview_editor->position = { 0, UIRootMenu::getHeight() };
            preview_editor->size     = { w->getSize().x / 2.0f, w->getSize().y - UIRootMenu::getHeight() };
        });
    view_layouts->addButton("top-to-bottom",
        [&]()
        {
            preview_editor->position = { 0, UIRootMenu::getHeight() };
            preview_editor->size     = { w->getSize().x, w->getSize().y / 2.0f };

            raw_editor->position = { 0, UIRootMenu::getHeight() + w->getSize().y / 2.0f };
            raw_editor->size     = { w->getSize().x, (w->getSize().y / 2.0f) - UIRootMenu::getHeight() };
        });

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

    UIButtonPalette* palette = new UIButtonPalette(1);
    palette->position = { (w->getSize().x - palette->size.x) - 16.0f, root_menu->getHeight() + 16.0f };
    for (int i = 0; i < 16; ++i) palette->addButton(i, [i]() { std::cout << i << std::endl; });

    while (!w->shouldClose())
    {
        // render
        w->setTitle((d->hasUnsavedChanges() ? "*" : "") +
                    (d->getPath().empty() ? "Untitled.aria" : d->getPath()) + " - ariaflow");
        w->makeCurrentContext();
        r->draw(w);
        r2->draw(w);
        w->present();
        w->poll();
        w->setCursorType(CURSOR_NORMAL);

        // build ui and check input
        r->clear();
        r2->clear();

        root_menu->checkInput(w);
        root_menu->draw(r, w->getSize().x);

        palette->checkInput(w);
        palette->draw(r);

        preview_editor->checkInput(w);
        preview_editor->draw(r);

        raw_editor->checkInput(w);
        raw_editor->draw(r);

        consumeAllMouseEvents(w);
        r->finalise();
        r2->finalise();
    }

    delete w;

    return EXIT_SUCCESS;
}