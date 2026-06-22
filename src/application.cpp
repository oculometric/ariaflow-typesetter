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
    while (true)
    {
        // render
        w->setTitle((d->getPath().empty() ? "Untitled.aria" : d->getPath()) +
                    (d->hasUnsavedChanges() ? "*" : "") + " - ariaflow");
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

        if (is_modal) right_click_open = false;
        if (right_click_open)
        {
            bool will_close = false;
            if (w->wasMouseReleased(MOUSE_LEFT, false)) will_close = true;
            right_click_menu->checkInput(w, right_click_position);
            right_click_menu->draw(r, right_click_position);
            w->wasMousePressed(MOUSE_LEFT);
            w->wasMouseReleased(MOUSE_LEFT);
            w->wasMousePressed(MOUSE_RIGHT);
            if (w->wasMouseReleased(MOUSE_RIGHT)) right_click_position = w->getMousePosition();
            if (will_close) right_click_open = false;
            w->setCursorType(CursorType::CURSOR_NORMAL, 4);
        }
        else if (!is_modal)
        {
            if (w->wasMouseReleased(MOUSE_RIGHT))
            {
                right_click_open     = true;
                right_click_position = w->getMousePosition();
            }
        }

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

            if (dialog_button_2->message.empty())
            {
                dialog_button_1->position =
                    glm::vec2{ (dialog_prompt->size.x * 0.5f) - (dialog_button_1->size.x / 2.0f),
                        dialog_prompt->size.y - (dialog_button_1->size.y + 12.0f) } +
                    dialog_prompt->position;

                dialog_button_1->checkInput(w);
                dialog_button_1->draw(r);
            }
            else
            {
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
        }

        if (w->wasShortcutTriggered("new_file") && !is_modal) fileNew();
        if (w->wasShortcutTriggered("open_file") && !is_modal) fileOpen();
        if (w->wasShortcutTriggered("export_file") && !is_modal) fileExport();
        if (w->wasShortcutTriggered("save_file") && !is_modal) fileSave();
        if (w->wasShortcutTriggered("save_as") && !is_modal) fileSaveAs();
        if (w->wasShortcutTriggered("save_incremental") && !is_modal) fileSaveIncremental();
        if (w->shouldClose() && !is_modal) fileQuit();

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
    right_click_open = false;
    right_click_menu = std::make_shared<UIMenu>();
    right_click_menu->addButton("copy", nullptr);
    right_click_menu->addButton("cut", nullptr);
    right_click_menu->addButton("paste", nullptr);
    right_click_menu->addDivider();
    right_click_menu->addSubMenu("insert");
    right_click_menu->addButton("refresh", nullptr);
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
    dialog_button_1->callback_func = [button_1_callback, this]()
    {
        if (button_1_callback) button_1_callback();
        is_modal = false;
    };
    dialog_button_1->z = 8.0f;

    if (!button_2.empty())
    {
        dialog_button_2->message       = button_2;
        dialog_button_2->icon_index    = button_2_icon;
        dialog_button_2->callback_func = [button_2_callback, this]()
        {
            if (button_2_callback) button_2_callback();
            is_modal = false;
        };
        dialog_button_2->z = 8.0f;
    }
    else
        dialog_button_2->message = "";
}