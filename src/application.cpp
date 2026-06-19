#include "application.h"

#include "document.h"
#include "user_interface.h"
#include "window.h"

#include <iostream>

using namespace AriaFlow;

Application::Application()
{
    initCore();
    initMenus();
    initEditors();
    initTools();

    dialog_prompt   = std::make_shared<UIResizablePanel>(glm::vec2{ 400, 200 });
    dialog_message  = std::make_shared<UILabel>("", TEXT_ALIGN_LEFT, TextFlags{});
    dialog_button_1 = std::make_shared<UIButton>("", []() {});
    dialog_button_2 = std::make_shared<UIButton>("", []() {});

    setSideBySideLayout();
    updateViewIcons();
}

void Application::run()
{
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

        edit_menu->setButtonDisabled(undo_button, !d->hasUndoStepsAvailable());
        edit_menu->setButtonDisabled(redo_button, !d->hasRedoStepsAvailable());

        if (!is_modal) root_menu->checkInput(w);
        root_menu->draw(r, w->getSize().x);

        if (show_palette)
        {
            palette->checkInput(w);
            palette->draw(r);
        }

        if (!is_modal) preview_editor->checkInput(w);
        preview_editor->draw(r);

        if (show_raw_editor)
        {
            if (!is_modal) raw_editor->checkInput(w);
            raw_editor->draw(r);
        }

        if (is_modal)
        {
            glm::vec2 temp          = dialog_prompt->size;
            dialog_prompt->position = (glm::vec2(w->getSize()) - dialog_prompt->size) / 2.0f;
            dialog_prompt->checkInput(w);
            dialog_prompt->size     = temp;
            dialog_prompt->position = (glm::vec2(w->getSize()) - dialog_prompt->size) / 2.0f;
            dialog_prompt->draw(r);
            dialog_prompt->size     = temp;
            dialog_prompt->position = (glm::vec2(w->getSize()) - dialog_prompt->size) / 2.0f;

            dialog_button_1->position =
                glm::vec2{ (dialog_prompt->size.x * 0.25f) - (dialog_button_1->size.x / 2.0f),
                    dialog_prompt->size.y - (dialog_button_1->size.y + 12.0f) } +
                dialog_prompt->position;
            dialog_button_2->position =
                glm::vec2{ (dialog_prompt->size.x * 0.75f) - (dialog_button_2->size.x / 2.0f),
                    dialog_prompt->size.y - (dialog_button_2->size.y + 12.0f) } +
                dialog_prompt->position;
            dialog_button_1->checkInput(w);
            dialog_button_2->checkInput(w);

            dialog_button_1->draw(r);
            dialog_button_2->draw(r);
        }

        w->clearMouseEvents();
        w->clearKeyEvents();
        w->clearCharEvents();
        r->finalise();
        r2->finalise();
    }
}

Application::~Application() {}

void Application::initCore()
{
    w = std::make_shared<Window>();
    w->makeCurrentContext();
    r  = std::make_shared<UIRenderer>();
    r2 = std::make_shared<UIRenderer>(1);
    d  = std::make_shared<Document>();
}

void Application::initMenus()
{
    root_menu = std::make_shared<UIRootMenu>();
    root_menu->addLabel("", 12);

    {
        file_menu = root_menu->addSubMenu("file");
        // TODO: check for unsaved changes!
        file_menu->addButton(
            "new",
            [&]() -> void
            {
                if (d->hasUnsavedChanges())
                {
                    triggerModal(
                        "unsaved changes",
                        "you have unsaved changes. do you want do discard them? they will be lost forever!",
                        7, { 320, 140 }, "cancel", [&]() { is_modal = false; }, -1, "discard",
                        [&]()
                        {
                            d                        = std::make_shared<Document>();
                            text_editor->data_source = d;
                            text_editor->refresh();
                            is_modal = false;
                        },
                        7);
                }
                else
                {
                    d                        = std::make_shared<Document>();
                    text_editor->data_source = d;
                }
            },
            "Ctrl+N");
        file_menu->addButton("open...", []() -> void { std::cout << "open" << std::endl; }, "Ctrl+O", 14);
        std::shared_ptr<UIMenu> recents = file_menu->addSubMenu("open recent");
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
    }

    {
        edit_menu = root_menu->addSubMenu("edit");
        edit_menu->addButton("copy", [&]() { w->triggerShortcut("copy"); }, "Ctrl+C");
        w->registerShortcut("copy", KeyEvent::CTRL, 'C');
        edit_menu->addButton("cut", [&]() { w->triggerShortcut("cut"); }, "Ctrl+X");
        w->registerShortcut("cut", KeyEvent::CTRL, 'X');
        edit_menu->addButton("paste", [&]() { w->triggerShortcut("paste"); }, "Ctrl+V");
        w->registerShortcut("paste", KeyEvent::CTRL, 'V');
        edit_menu->addDivider();
        undo_button = edit_menu->addButton("undo", [&]() { w->triggerShortcut("undo"); }, "Ctrl+Z");
        w->registerShortcut("undo", KeyEvent::CTRL, 'Z');
        redo_button = edit_menu->addButton("redo", [&]() { w->triggerShortcut("redo"); }, "Ctrl+Shft+Z");
        w->registerShortcut("redo", (KeyEvent::Modifier)(KeyEvent::CTRL | KeyEvent::SHIFT), 'Z');
        edit_menu->addDivider();
        edit_menu->addButton("select all", [&]() { w->triggerShortcut("select_all"); }, "Ctrl+A");
        w->registerShortcut("select_all", KeyEvent::CTRL, 'A');
        edit_menu->addButton(
            "select paragraph", [&]() { w->triggerShortcut("select_paragraph"); }, "Ctrl+Shft+A");
        w->registerShortcut("select_paragraph", (KeyEvent::Modifier)(KeyEvent::CTRL | KeyEvent::SHIFT),
            'A');
        edit_menu->addDivider();
        edit_menu->addButton("format", nullptr, "Alt+Shft+F");
        w->registerShortcut("format", (KeyEvent::Modifier)(KeyEvent::ALT | KeyEvent::SHIFT), 'F');
        edit_menu->addDivider();
        edit_menu->addButton("settings", nullptr, "Ctrl+,");
        w->registerShortcut("settings", KeyEvent::CTRL, ',');
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
        w->registerShortcut("toggle_metrics", KeyEvent::ALT, 'M');
        view_menu->addButton("show guides", nullptr, "Alt+G");
        w->registerShortcut("toggle_guides", KeyEvent::ALT, 'G');
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

void Application::initEditors()
{
    text_editor              = std::make_shared<UITextEditor>();
    text_editor->custom_r    = r2;
    text_editor->data_source = d;

    raw_editor                    = std::make_shared<UIResizablePanel>();
    raw_editor->title             = "raw view";
    raw_editor->child             = text_editor;
    raw_editor->button_a_icon     = 0;
    raw_editor->button_b_icon     = 2;
    raw_editor->button_a_callback = [&]()
    {
        show_raw_editor = !show_raw_editor;
        updateViewIcons();
    };

    preview_editor                = std::make_shared<UIResizablePanel>();
    preview_editor->title         = "rendered view";
    preview_editor->button_b_icon = -1;
    preview_editor->button_a_icon = 2;
}

void Application::initTools()
{
    show_palette = true;
    palette      = std::make_shared<UIButtonPalette>(1);
    for (int i = 0; i < 16; ++i) palette->addButton(i, [i]() { std::cout << i << std::endl; });
}

void Application::updateViewIcons()
{
    view_menu->setButtonIcon(0, show_palette ? 9 : -1);
    view_menu->setButtonIcon(1, show_raw_editor ? 9 : -1);
}

void Application::setSideBySideLayout()
{
    show_palette      = true;
    palette->position = { (w->getSize().x - palette->size.x) - 4.0f, UIRootMenu::getHeight() + 4.0f };

    show_raw_editor      = true;
    raw_editor->position = { w->getSize().x / 2.0f, UIRootMenu::getHeight() };
    raw_editor->size     = { (w->getSize().x / 2.0f) - (palette->size.x + 8.0f),
        w->getSize().y - UIRootMenu::getHeight() };

    preview_editor->position = { 0, UIRootMenu::getHeight() };
    preview_editor->size     = { w->getSize().x / 2.0f, w->getSize().y - UIRootMenu::getHeight() };
    updateViewIcons();
}

void Application::setTopToBottomLayout()
{
    show_palette      = true;
    palette->position = { (w->getSize().x - palette->size.x) - 4.0f, UIRootMenu::getHeight() + 4.0f };

    preview_editor->position = { 0, UIRootMenu::getHeight() };
    preview_editor->size     = { w->getSize().x, w->getSize().y / 2.0f };

    show_raw_editor      = true;
    raw_editor->position = { 0, UIRootMenu::getHeight() + w->getSize().y / 2.0f };
    raw_editor->size     = { w->getSize().x, (w->getSize().y / 2.0f) - UIRootMenu::getHeight() };
    updateViewIcons();
}

void Application::triggerModal(const std::string& title, const std::string& message, int message_icon,
    glm::vec2 size, const std::string& button_1, std::function<void(void)> button_1_callback,
    int button_1_icon, const std::string& button_2, std::function<void(void)> button_2_callback,
    int button_2_icon)
{
    is_modal                     = true;
    dialog_prompt->title         = title;
    dialog_prompt->size          = size;
    dialog_prompt->minimum_size  = size;
    dialog_prompt->button_a_icon = -1;
    dialog_prompt->button_b_icon = -1;
    dialog_prompt->z             = 8.0f;
    dialog_prompt->child         = dialog_message;

    dialog_message->message    = message;
    dialog_message->z          = 8.0f;
    dialog_message->settings   = TEXT_FLAGS_BOLD;
    dialog_message->direction  = TEXT_ALIGN_CENTER;
    dialog_message->wrap       = true;
    dialog_message->text_size  = 20.0f;
    dialog_message->icon_index = message_icon;

    dialog_button_1->message       = button_1;
    dialog_button_1->icon_index    = button_1_icon;
    dialog_button_1->callback_func = button_1_callback;
    dialog_button_1->z             = 8.0f;

    if (!button_2.empty())
    {
        dialog_button_2->message       = button_2;
        dialog_button_2->icon_index    = button_2_icon;
        dialog_button_2->callback_func = button_2_callback;
        dialog_button_2->z             = 8.0f;
    }
    else
        dialog_button_2->message = "";
}
