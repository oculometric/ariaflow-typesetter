#include "application.h"
#include "document.h"
#include "user_interface.h"
#include "window.h"

#include <filesystem>
#include <iostream>
#include <portable-file-dialogs/portable-file-dialogs.h>

using namespace AriaFlow;

void Application::initMenus()
{
    root_menu = std::make_shared<UIRootMenu>();
    root_menu->addLabel("", 12);

    {
        file_menu = root_menu->addSubMenu("file");
        file_menu->addButton("new", [&]() -> void { fileNew(); }, "Ctrl+N");
        w->registerShortcut("new_file", MODIFIER_CTRL, 'N');
        file_menu->addButton("open...", [&]() -> void { fileOpen(); }, "Ctrl+O", 14);
        w->registerShortcut("open_file", MODIFIER_CTRL, 'O');
        std::shared_ptr<UIMenu> recents = file_menu->addSubMenu("open recent");
        // TODO: open recents history
        recents->addButton("item 1", []() -> void { std::cout << "test" << std::endl; });
        recents->addButton("item 2", nullptr);
        recents->addButton("item 3", nullptr);
        recents->addButton("item 4", nullptr);
        file_menu->addDivider();
        file_menu->addButton("export...", [&]() -> void { fileExport(); }, "Ctrl+E", 13);
        w->registerShortcut("export_file", MODIFIER_CTRL, 'E');
        file_menu->addButton("save", [&]() -> void { fileSave(); }, "Ctrl+S", 11);
        w->registerShortcut("save_file", MODIFIER_CTRL, 'S');
        file_menu->addButton("save as...", [&]() -> void { fileSaveAs(); }, "Ctrl+Shft+S", 11);
        w->registerShortcut("save_as", MODIFIER_CTRL | MODIFIER_SHIFT, 'S');
        file_menu->addButton(
            "save incremental", [&]() -> void { fileSaveIncremental(); }, "Ctrl+Alt+S", 11);
        w->registerShortcut("save_incremental", MODIFIER_CTRL | MODIFIER_ALT, 'S');
        file_menu->addButton("revert", [&]() -> void { fileRevert(); }, "", 15);
        file_menu->addDivider();
        file_menu->addButton("exit", [&]() -> void { fileQuit(); }, "Alt+F4");
    }

    {
        edit_menu = root_menu->addSubMenu("edit");
        edit_menu->addButton("copy", [&]() { w->triggerShortcut("copy"); }, "Ctrl+C");
        w->registerShortcut("copy", MODIFIER_CTRL, 'C');
        edit_menu->addButton("cut", [&]() { w->triggerShortcut("cut"); }, "Ctrl+X");
        w->registerShortcut("cut", MODIFIER_CTRL, 'X');
        edit_menu->addButton("paste", [&]() { w->triggerShortcut("paste"); }, "Ctrl+V");
        w->registerShortcut("paste", MODIFIER_CTRL, 'V');
        edit_menu->addDivider();
        undo_button = edit_menu->addButton("undo", [&]() { w->triggerShortcut("undo"); }, "Ctrl+Z");
        w->registerShortcut("undo", MODIFIER_CTRL, 'Z');
        redo_button = edit_menu->addButton("redo", [&]() { w->triggerShortcut("redo"); }, "Ctrl+Shft+Z");
        w->registerShortcut("redo", MODIFIER_CTRL | MODIFIER_SHIFT, 'Z');
        edit_menu->addDivider();
        edit_menu->addButton("select all", [&]() { w->triggerShortcut("select_all"); }, "Ctrl+A");
        w->registerShortcut("select_all", MODIFIER_CTRL, 'A');
        edit_menu->addButton(
            "select paragraph", [&]() { w->triggerShortcut("select_paragraph"); }, "Ctrl+Shft+A");
        w->registerShortcut("select_paragraph", MODIFIER_CTRL | MODIFIER_SHIFT, 'A');
        edit_menu->addDivider();
        edit_menu->addButton("format", nullptr, "Alt+Shft+F");
        w->registerShortcut("format", MODIFIER_ALT | MODIFIER_SHIFT, 'F');
        edit_menu->addDivider();
        edit_menu->addButton("settings", nullptr, "Ctrl+,");
        w->registerShortcut("settings", MODIFIER_CTRL, ',');
    }

    {
        scripts_menu = root_menu->addSubMenu("scripts");
        scripts_menu->addLabel("you have no scripts.", 7);
    }

    {
        view_menu = root_menu->addSubMenu("view");
        view_menu->addButton(
            "show palette",
            [&]()
            {
                show_palette = !show_palette;
                updateViewIcons();
            },
            "Alt+P");
        view_menu->addButton(
            "show raw view",
            [&]()
            {
                show_raw_editor = !show_raw_editor;
                updateViewIcons();
            },
            "Alt+R");
        view_menu->addButton("show metrics", nullptr, "Alt+M");
        w->registerShortcut("toggle_metrics", MODIFIER_ALT, 'M');
        view_menu->addButton("show guides", nullptr, "Alt+G");
        w->registerShortcut("toggle_guides", MODIFIER_ALT, 'G');
        view_menu->addDivider();
        std::shared_ptr<UIMenu> view_layouts = view_menu->addSubMenu("reset layout", 15);
        view_layouts->addButton("side-by-side", [&]() { setSideBySideLayout(); });
        view_layouts->addButton("top-to-bottom", [&]() { setTopToBottomLayout(); });
        view_menu->addButton("swap layout",
            [&]() -> void
            {
                glm::vec2 tmp            = raw_editor->position;
                glm::vec2 tmp2           = raw_editor->size;
                raw_editor->position     = preview_editor->position;
                raw_editor->size         = preview_editor->size;
                preview_editor->position = tmp;
                preview_editor->size     = tmp2;
            });
    }

    {
        std::shared_ptr<UIMenu> help_menu = root_menu->addSubMenu("help");
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
    }

    root_menu->addButton("test", []() -> void { std::cout << "test" << std::endl; });
    updateViewIcons();
}

void Application::fileNew()
{
    auto finalCode = [&]() -> void
    {
        d = std::make_shared<Document>();
        d->getData().clear();
        text_editor->data_source = d;
        text_editor->refresh();
    };
    if (d->hasUnsavedChanges())
    {
        triggerModal("unsaved changes",
            "you have unsaved changes. do you want do discard them? they will be lost forever!", 7,
            { 320, 140 }, "cancel", nullptr, -1, "discard", finalCode, 7);
    }
    else
        finalCode();
}

void Application::fileOpen()
{
    auto dialogCode = [&]() -> void
    {
        auto open_dialog = pfd::open_file("export...", "",
            { "AriaFlow Files (.aria)", "*.aria", "Text Files (.txt)", "*.txt" }, pfd::opt::none);
        if (!open_dialog.result().empty())
        {
            d                        = std::make_shared<Document>(open_dialog.result()[0]);
            text_editor->data_source = d;
            text_editor->refresh();
        }
    };
    if (d->hasUnsavedChanges())
    {
        triggerModal("unsaved changes",
            "you have unsaved changes. do you want do discard them? they will be lost forever!", 7,
            { 320, 140 }, "cancel", nullptr, -1, "discard", dialogCode, 7);
    }
    else
        dialogCode();
}

void Application::fileExport()
{
    auto export_dialog = pfd::save_file("export...", (d->getPath().empty() ? "~/Export.pdf" : d->getPath()),
        { "PDF Files (.pdf)", "*.pdf", "Image Files (.png, .jpg, .jpeg)", "*.png *.jpg *.jpeg",
            "HTML Files (.html)", "*.html", "Markdown Files (.md)", "*.md", "Text Files (.txt)", "*.txt",
            "All Files (.pdf, .png, .jpg, .jpeg, .html, .md, .txt)",
            "*.pdf *.png *.jpg *.jpeg *.html *.md *.txt" },
        pfd::opt::none);
    if (export_dialog.result().empty()) return;

    triggerModal("unimplemented", "i actually don't know how to export stuff yet.", 10, { 320, 140 },
        "okay", nullptr, -1, "", nullptr, -1);

    // TODO: export (pdf, images, html, markdown, plain text)
    std::cout << "fileExport UNIMPLEMENTED" << std::endl;
}

void Application::fileSave()
{
    if (!d->save()) fileSaveAs();
}

void Application::fileSaveAs()
{
    bool flag = false;
    do
    {
        auto save_dialog =
            pfd::save_file("save as...", (d->getPath().empty() ? "~/Untitled.aria" : d->getPath()),
                { "AriaFlow Files (.aria)", "*.aria", "Text Files (.txt)", "*.txt" }, pfd::opt::none);
        if (save_dialog.result().empty()) break;
        flag = d->saveAs(save_dialog.result());
    } while (!flag);
}

void Application::fileSaveIncremental()
{
    if (!d->saveIncremental()) fileSaveAs();
}

void Application::fileRevert()
{
    if (d->hasUnsavedChanges())
    {
        triggerModal(
            "unsaved changes", "you have unsaved changes. are you sure you want to revert them?", 7,
            { 320, 140 }, "cancel", nullptr, -1, "revert",
            [&]()
            {
                d->revert();
                text_editor->refresh();
            },
            7);
    }
    else
    {
        d->revert();
        text_editor->refresh();
    }
}

void Application::fileQuit()
{
    if (d->hasUnsavedChanges())
    {
        triggerModal(
            "unsaved changes",
            "you have unsaved changes. do you want do discard them? they will be lost forever!", 7,
            { 320, 140 }, "cancel", nullptr, -1, "exit", [&]() { exit(EXIT_SUCCESS); }, 7);
    }
    else
        exit(EXIT_SUCCESS);
}
