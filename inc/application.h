#pragma once

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
};

} // namespace AriaFlow