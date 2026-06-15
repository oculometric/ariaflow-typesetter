#include "user_interface.h"

using namespace AriaFlow;

UIButtonPalette::UIButtonPalette(int button_columns, glm::vec2 offset)
{
    columns          = button_columns;
    position         = offset;
    UIButton* button = new UIButton("", nullptr, 0);
    button_size      = button->getSize(nullptr);
    delete button;
    size          = recalculateSize();
    grabbables[0] = new UIGrabbable(CURSOR_HAND);
    grabbables[1] = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);
    grabbables[2] = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);
}

UIButtonPalette::~UIButtonPalette()
{
    for (auto g : grabbables) delete g;
    for (auto b : buttons) delete b;
}

UIButton* UIButtonPalette::addButton(int icon, std::function<void(void)> callback)
{
    UIButton* b = new UIButton("", callback, icon);
    buttons.push_back(b);
    b->z = 8.2f;
    return b;
}

void UIButtonPalette::draw(UIRenderer* r)
{
    glm::vec2 panel_position   = glm::round(position);
    glm::vec2 panel_size       = glm::round(size);
    glm::vec2 content_position = panel_position + glm::vec2{ small_border, medium_border };
    glm::vec2 content_size     = glm::round(calculateButtonArea());
    r->addNineSlice(panel_position, 8.1f, panel_size, 1, { 0.4f, 0.4f, 0.4f, 1.0f }, 0b1111);

    int col      = 0;
    float height = 0;
    for (auto button : buttons)
    {
        button->position = content_position + glm::vec2{ button_size.x * col, height };
        button->draw(r);
        if ((col + 1) % columns == 0) height += button_size.y;
        col = (col + 1) % columns;
    }

    r->addSimple(panel_position + glm::vec2{ (panel_size.x / 2.0f) - (medium_border / 2.0f), 0 }, 8.2f,
        { medium_border, medium_border }, 1, { 0, 0 }, { 1, 1 });
}

void UIButtonPalette::checkInput(Window* w)
{
    trackWindowResizeFixedSize(w, position, size);

    position = grabbables[0]->checkInput(w, position, { size.x, medium_border });

    glm::vec2 grab_pos = position + glm::vec2{ 0, medium_border };
    glm::vec2 palette_left =
        grabbables[1]->checkInput(w, grab_pos, { small_border, size.y - (small_border + medium_border) });
    float change = palette_left.x - grab_pos.x;
    change       = size.x - glm::clamp(size.x - change, button_size.x + (small_border * 2),
                                static_cast<float>(button_size.x * buttons.size()) + (small_border * 2));
    position.x += change;
    size.x -= change;

    grab_pos = position + glm::vec2{ size.x - small_border, medium_border };
    glm::vec2 palette_right =
        grabbables[2]->checkInput(w, grab_pos, { small_border, size.y - (small_border + medium_border) });
    change = grab_pos.x - palette_right.x;
    change = size.x - glm::clamp(size.x - change, button_size.x + (small_border * 2),
                          static_cast<float>(button_size.x * buttons.size()) + (small_border * 2));
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
    glm::vec2 content_position = panel_position + glm::vec2{ small_border, medium_border };
    glm::vec2 content_size     = glm::round(calculateButtonArea());

    int col      = 0;
    float height = 0;
    for (auto button : buttons)
    {
        button->position = content_position + glm::vec2{ button_size.x * col, height };
        button->checkInput(w);
        if ((col + 1) % columns == 0) height += button_size.y;
        col = (col + 1) % columns;
    }
}

glm::vec2 UIButtonPalette::recalculateSize()
{
    return { (button_size.x * columns) + (small_border * 2),
        (button_size.y * glm::ceil(static_cast<float>(buttons.size()) / static_cast<float>(columns))) +
            medium_border + small_border };
}

glm::vec2 UIButtonPalette::calculateButtonArea()
{ return size - glm::vec2{ (small_border * 2), (medium_border + small_border) }; }
