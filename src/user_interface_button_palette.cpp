#include "user_interface.h"

using namespace AriaFlow;

UIButtonPalette::UIButtonPalette()
{
    position         = { 0, 0 };
    UIButton* button = new UIButton("", nullptr, 0);
    button_size      = button->getSize(nullptr);
    delete button;
    size          = recalculateSize();
    panel         = new UIPanel({ 0.4f, 0.4f, 0.4f, 1.0f }, 1, 0b1111);
    grabbables[0] = new UIGrabbable(CURSOR_HAND);
    grabbables[1] = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);
    grabbables[2] = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);
}

UIButtonPalette::~UIButtonPalette()
{
    delete panel;
    for (auto g : grabbables) delete g;
    for (auto b : buttons) delete b;
}

UIButton* UIButtonPalette::addButton(int icon, std::function<void(void)> callback)
{
    UIButton* b = new UIButton("", callback, icon);
    buttons.push_back(b);
    return b;
}

void UIButtonPalette::draw(UIRenderer* r)
{
    glm::vec2 panel_position   = glm::round(position);
    glm::vec2 panel_size       = glm::round(size);
    glm::vec2 content_position = panel_position + glm::vec2{ 4, 12 };
    glm::vec2 content_size     = glm::round(calculateButtonArea());
    panel->draw(r, panel_position, panel_size);

    int col      = 0;
    float height = 0;
    for (auto button : buttons)
    {
        button->draw(r, content_position + glm::vec2{ button_size.x * col, height });
        if ((col + 1) % columns == 0) height += button_size.y;
        col = (col + 1) % columns;
    }
}

void AriaFlow::trackWindowResize(Window* w, glm::vec2& top_left, glm::vec2 size)
{
    glm::vec2 old_window_size = w->getLastSize();
    glm::vec2 new_window_size = w->getSize();

    glm::vec2 midpoint     = top_left + (size / 2.0f);
    glm::vec2 bottom_right = top_left + size;

    if (midpoint.x > old_window_size.x / 2.0f) top_left.x += (new_window_size - old_window_size).x;

    if ((glm::abs(midpoint.y - (old_window_size.y / 2.0f)) < (old_window_size.y / 8.0f)) &&
        !(top_left.y < 100.0f || bottom_right.y > old_window_size.y - 100.0f))
        top_left.y += (new_window_size - old_window_size).y / 2.0f;
    else if (midpoint.y > old_window_size.y / 2.0f)
        top_left.y += (new_window_size - old_window_size).y;
}

void UIButtonPalette::checkInput(Window* w)
{
    trackWindowResize(w, position, size);

    position = grabbables[0]->checkInput(w, position, { size.x, 12 });

    glm::vec2 grab_pos     = position + glm::vec2{ 0, 4 };
    glm::vec2 palette_left = grabbables[1]->checkInput(w, grab_pos, { 4, size.y - 4 });
    float change           = palette_left.x - grab_pos.x;
    change                 = size.x - glm::clamp(size.x - change, button_size.x + 4 + 4,
                                          static_cast<float>(button_size.x * buttons.size()) + 4 + 4);
    position.x += change;
    size.x -= change;

    grab_pos                = position + glm::vec2{ size.x - 4, 4 };
    glm::vec2 palette_right = grabbables[2]->checkInput(w, grab_pos, { 4, size.y - 4 });
    change                  = grab_pos.x - palette_right.x;
    change                  = size.x - glm::clamp(size.x - change, button_size.x + 4 + 4,
                                           static_cast<float>(button_size.x * buttons.size()) + 4 + 4);
    size.x -= change;

    columns = glm::min(glm::max(1, static_cast<int>(glm::floor(calculateButtonArea().x / button_size.x))),
        static_cast<int>(buttons.size()));
    size.y  = recalculateSize().y;
    if (!grabbables[1]->isCurrentlyGrabbed() && !grabbables[2]->isCurrentlyGrabbed())
    {
        size.x   = recalculateSize().x;
        position = glm::clamp(position, { 0, UIRootMenu::getHeight() }, glm::vec2(w->getSize()) - size);
    }

    glm::vec2 panel_position   = glm::round(position);
    glm::vec2 panel_size       = glm::round(size);
    glm::vec2 content_position = panel_position + glm::vec2{ 4, 12 };
    glm::vec2 content_size     = glm::round(calculateButtonArea());

    int col      = 0;
    float height = 0;
    for (auto button : buttons)
    {
        button->checkInput(w, content_position + glm::vec2{ button_size.x * col, height });
        if ((col + 1) % columns == 0) height += button_size.y;
        col = (col + 1) % columns;
    }
}

glm::vec2 UIButtonPalette::recalculateSize()
{
    return { (button_size.x * columns) + 4 + 4,
        (button_size.y * glm::ceil(static_cast<float>(buttons.size()) / static_cast<float>(columns))) + 12 +
            4 };
}

glm::vec2 UIButtonPalette::calculateButtonArea() { return size - glm::vec2{ 4 + 4, 12 + 4 }; }
