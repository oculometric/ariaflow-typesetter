#include "user_interface.h"
#include "window.h"

using namespace AriaFlow;

UIResizablePanel::UIResizablePanel(glm::vec2 min_size, glm::vec2 offset, glm::vec2 dimensions)
{
    minimum_size  = min_size;
    position      = offset;
    size          = dimensions;
    grabbables[0] = new UIGrabbable(CURSOR_HAND);
    grabbables[1] = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);
    grabbables[2] = new UIGrabbable(CURSOR_RESIZE_HORIZONTAL);
    grabbables[3] = new UIGrabbable(CURSOR_RESIZE_VERTICAL);
    grabbables[4] = new UIGrabbable(CURSOR_RESIZE_VERTICAL);
    grabbables[5] = new UIGrabbable(CURSOR_RESIZE_TLBR);
    grabbables[6] = new UIGrabbable(CURSOR_RESIZE_BLTR);
    grabbables[7] = new UIGrabbable(CURSOR_RESIZE_BLTR);
    grabbables[8] = new UIGrabbable(CURSOR_RESIZE_TLBR);
}

UIResizablePanel::~UIResizablePanel()
{
    for (auto g : grabbables) delete g;
}

void UIResizablePanel::draw(UIRenderer* r)
{
    glm::vec2 panel_position = glm::round(position);
    glm::vec2 panel_size     = glm::round(size);
    r->addNineSlice(panel_position, z, panel_size, 1, panel_colour, 0b1111);
    glm::vec2 content_position;
    glm::vec2 content_size;
    calculateContentArea(content_position, content_size);

    TextFormatting format;
    format.align = TEXT_ALIGN_CENTER;
    format.flags = TEXT_FLAGS_BOLD;
    format.size  = medium_border + 1.5f;
    r->addText(panel_position + glm::vec2{ panel_size.x / 2.0f, text_push + small_border + 0.5f }, z,
        format, title, text_colour);

    r->addNineSlice({ panel_position.x + (spacing * 4), content_position.y - 2 }, z,
        { content_size.x - (spacing * 8), 2 }, 3, panel_sec_colour, 0b0001);

    if (button_a_icon != -1)
    {
        r->addNineSlice({ panel_position.x + panel_size.x - 16.0f - spacing, panel_position.y + spacing },
            z, { 16.0f, 16.0f }, button_a_down ? 2 : 0, panel_colour, 0b1111);
        r->addSimple(
            { panel_position.x + panel_size.x - 14.0f - spacing, panel_position.y + 2.0f + spacing }, z,
            { 12.0f, 12.0f }, button_a_icon, { 0, 0 }, { 1, 1 });
    }
    if (button_b_icon != -1)
    {
        r->addNineSlice({ panel_position.x + panel_size.x - 32.0f - spacing, panel_position.y + spacing },
            z, { 16.0f, 16.0f }, button_b_down ? 2 : 0, panel_colour, 0b1111);
        r->addSimple(
            { panel_position.x + panel_size.x - 30.0f - spacing, panel_position.y + 2.0f + spacing }, z,
            { 12.0f, 12.0f }, button_b_icon, { 0, 0 }, { 1, 1 });
    }

    if (child)
    {
        child->size     = content_size;
        child->position = content_position;
        child->z        = z + 1.0f;
        child->draw(r);
    }
    else
        r->addNineSlice(content_position, z, content_size, 3, { 0.01f, 0.01f, 0.01f, 1.0f }, 0b1111);
}

void UIResizablePanel::checkInput(Window* w)
{
    trackWindowResizeScaleSize(w, position, size);

    glm::vec2 panel_position = glm::round(position);
    glm::vec2 panel_size     = glm::round(size);
    bool skip_resize         = false;
    if (insideRect(w->getMousePosition(),
            { panel_position.x + panel_size.x - 16.0f - spacing, panel_position.y + spacing },
            { 16.0f, 16.0f }))
    {
        skip_resize = true;
        if (button_a_down && checkForMouseUp(w) && button_a_callback != nullptr) button_a_callback();
        if (!button_a_down && checkForMouseDown(w)) button_a_down = true;
    }
    if (insideRect(w->getMousePosition(),
            { panel_position.x + panel_size.x - 32.0f - spacing, panel_position.y + spacing },
            { 16.0f, 16.0f }))
    {
        skip_resize = true;
        if (button_b_down && checkForMouseUp(w) && button_b_callback != nullptr) button_b_callback();
        if (!button_b_down && checkForMouseDown(w)) button_b_down = true;
    }
    if (button_a_down || button_b_down) skip_resize = true;
    if (!w->isMouseDown(KeyEvent::MOUSE_LEFT))
    {
        button_a_down = false;
        button_b_down = false;
    }

    if (!skip_resize)
        position = grabbables[0]->checkInput(w, position + small_border,
                       glm::vec2{ size.x - (small_border * 2), medium_border }) -
                   small_border;

    if (!skip_resize)
    {
        glm::vec2 max_size = glm::vec2(10000.0f);
        // glm::vec2(w->getSize()) - glm::vec2{ 0, UIRootMenu::getHeight() };

        glm::vec2 change_size     = { 0, 0 };
        glm::vec2 change_position = { 0, 0 };
        glm::vec2 grab_pos        = position + glm::vec2{ 0, small_border };
        glm::vec2 left_grab =
            grabbables[1]->checkInput(w, grab_pos, glm::vec2{ small_border, size.y - (small_border * 2) });
        float change = left_grab.x - grab_pos.x;
        change       = size.x - glm::clamp(size.x - change, minimum_size.x, max_size.x);
        change_size.x += change;
        change_position.x += change;

        grab_pos = position + glm::vec2{ size.x - small_border, small_border };
        glm::vec2 right_grab =
            grabbables[2]->checkInput(w, grab_pos, glm::vec2{ small_border, size.y - (small_border * 2) });
        change = right_grab.x - grab_pos.x;
        change_size.x -= change;

        grab_pos = position + glm::vec2{ small_border, 0 };
        glm::vec2 top_grab =
            grabbables[3]->checkInput(w, grab_pos, glm::vec2{ size.x - (small_border * 2), small_border });
        change = top_grab.y - grab_pos.y;
        change = size.y - glm::clamp(size.y - change, minimum_size.y, max_size.y);
        change_size.y += change;
        change_position.y += change;

        grab_pos = position + glm::vec2{ small_border, size.y - small_border };
        glm::vec2 bottom_grab =
            grabbables[4]->checkInput(w, grab_pos, glm::vec2{ size.x - (small_border * 2), small_border });
        change = bottom_grab.y - grab_pos.y;
        change_size.y -= change;

        grab_pos          = position;
        glm::vec2 tl_grab = grabbables[5]->checkInput(w, grab_pos, glm::vec2{ small_border, small_border });
        glm::vec2 change2 = tl_grab - grab_pos;
        change2           = size - glm::clamp(size - change2, minimum_size, max_size);
        change_size += change2;
        change_position += change2;

        grab_pos          = position + glm::vec2{ size.x - small_border, 0 };
        glm::vec2 tr_grab = grabbables[6]->checkInput(w, grab_pos, glm::vec2{ small_border, small_border });
        change2           = tr_grab - grab_pos;
        change2.y         = size.y - glm::clamp(size.y - change2.y, minimum_size.y, max_size.y);
        change_size += change2 * glm::vec2{ -1, 1 };
        change_position.y += change2.y;

        grab_pos          = position + glm::vec2{ 0, size.y - small_border };
        glm::vec2 bl_grab = grabbables[7]->checkInput(w, grab_pos, glm::vec2{ small_border, small_border });
        change2           = bl_grab - grab_pos;
        change2.x         = size.x - glm::clamp(size.x - change2.x, minimum_size.x, max_size.x);
        change_size += change2 * glm::vec2{ 1, -1 };
        change_position.x += change2.x;

        grab_pos          = position + glm::vec2{ size.x - small_border, size.y - small_border };
        glm::vec2 br_grab = grabbables[8]->checkInput(w, grab_pos, glm::vec2{ small_border, small_border });
        change2           = br_grab - grab_pos;
        change_size -= change2;

        position += change_position;
        size -= change_size;
    }

    position =
        glm::clamp(position, { 16.0f - size.x, UIRootMenu::getHeight() }, glm::vec2(w->getSize()) - 16.0f);
    size = glm::clamp(size, minimum_size, glm::vec2(10000.0f));

    glm::vec2 content_position;
    glm::vec2 content_size;
    calculateContentArea(content_position, content_size);
    if (child)
    {
        child->size     = content_size;
        child->position = content_position;
        child->checkInput(w);
    }

    // TODO: snapping to window when released?
}

void UIResizablePanel::calculateContentArea(glm::vec2& offset, glm::vec2& dimensions)
{
    offset     = glm::round(this->position + glm::vec2{ small_border, medium_border + small_border });
    dimensions = glm::round(
        this->size - glm::vec2{ small_border + small_border, medium_border + small_border + small_border });
}
