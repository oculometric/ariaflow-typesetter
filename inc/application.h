#pragma once

#include <functional>
#include <glm/vec2.hpp>
#include <memory>

namespace AriaFlow
{

class Document;
class Window;
class UIRenderer;
class UIMenu;
class UIRootMenu;
class UIElement;
class UIButton;
class UILabel;
class UIPanel;
class UIGrabbable;
class UIButtonPalette;
class UIResizablePanel;
class UITextEditor;

class Application
{
private:
    std::shared_ptr<Window> w;
    std::shared_ptr<UIRenderer> r;
    std::shared_ptr<UIRenderer> r2;
    std::shared_ptr<Document> d;

    // menus
    std::shared_ptr<UIRootMenu> root_menu;
    std::shared_ptr<UIMenu> file_menu;
    std::shared_ptr<UIMenu> edit_menu;
    size_t undo_button;
    size_t redo_button;
    std::shared_ptr<UIMenu> scripts_menu;
    std::shared_ptr<UIMenu> view_menu;
    std::shared_ptr<UIMenu> help_menu;

    // editor panels
    bool show_raw_editor;
    std::shared_ptr<UIResizablePanel> raw_editor;
    std::shared_ptr<UITextEditor> text_editor;
    std::shared_ptr<UIResizablePanel> preview_editor;

    // tool panels
    bool show_palette;
    std::shared_ptr<UIButtonPalette> palette;
    // TODO: properties panel
    // TODO: metrics panel
    // TODO: document panel
    bool right_click_open;
    glm::vec2 right_click_position; // TODO: right click menu
    std::shared_ptr<UIMenu> right_click_menu;

    // modal panels
    bool is_modal = false;
    std::shared_ptr<UIResizablePanel> dialog_prompt;
    std::shared_ptr<UIButton> dialog_button_1;
    std::shared_ptr<UIButton> dialog_button_2;
    std::shared_ptr<UILabel> dialog_message;

public:
    Application();
    void run();
    ~Application();

private:
    void initCore();
    void initMenus();
    void initEditors();
    void initTools();

    void updateViewIcons();
    void setSideBySideLayout();
    void setTopToBottomLayout();

    void triggerModal(const std::string& title, const std::string& message, int message_icon,
        glm::vec2 size, const std::string& button_1, std::function<void(void)> button_1_callback,
        int button_1_icon, const std::string& button_2, std::function<void(void)> button_2_callback,
        int button_2_icon);

    void fileNew();
    void fileOpen();
    void fileExport();
    void fileSave();
    void fileSaveAs();
    void fileSaveIncremental();
    void fileRevert();
    void fileQuit();
};

} // namespace AriaFlow