#include "user_interface.h"
#include "window.h"

using namespace AriaFlow;

UITextEditor::UITextEditor()
{
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

UITextEditor::~UITextEditor()
{
    for (auto g : grabbables) delete g;
}

void UITextEditor::draw(UIRenderer* r, glm::vec2 position, glm::vec2 size)
{
    position = glm::round(position);
    size     = glm::round(size);
    r->addNineSlice(position, -1, size, 1, panel_colour, 0b1111);
    glm::vec2 content_position = position + glm::vec2{ 4, 12 };
    glm::vec2 content_size     = size - glm::vec2{ 4 + 4, 12 + 4 };
    // TEMPORARY
    r->addNineSlice(content_position, -1, content_size, 3, { 0.01f, 0.01f, 0.01f, 1.0f }, 0b1111);
}

const float editor_min_width = 64;

void UITextEditor::checkInput(Window* w, glm::vec2& position, glm::vec2& size)
{
    {
        glm::vec2 old_window_size = w->getLastSize();
        glm::vec2 new_window_size = w->getSize();

        glm::vec2 top_left     = position;
        glm::vec2 bottom_right = position + size;
        glm::vec2 top_right    = position + glm::vec2{ size.x, 0 };
        glm::vec2 bottom_left  = position + glm::vec2{ 0, size.y };

        auto scaleRelative = [old_window_size, new_window_size](glm::vec2 point) -> glm::vec2
        {
            bool left   = point.x < old_window_size.x / 2.0f;
            bool center = glm::abs(point.x - (old_window_size.x / 2.0f)) < (old_window_size.x / 4.0f);
            bool top    = point.y < old_window_size.y / 2.0f;
            bool middle = glm::abs(point.y - (old_window_size.y / 2.0f)) < (old_window_size.y / 4.0f);
            auto pick   = [&](glm::vec2 size) -> glm::vec2
            {
                return { center ? size.x / 2.0f : (left ? 0 : size.x),
                    middle ? size.y / 2.0f : (top ? 0 : size.y) };
            };
            glm::vec2 origin     = pick(old_window_size);
            glm::vec2 new_origin = pick(new_window_size);
            return point + (new_origin - origin);
        };
        top_left     = scaleRelative(top_left);
        bottom_right = scaleRelative(bottom_right);
        position     = top_left;
        size         = bottom_right - top_left;
    }

    position = grabbables[0]->checkInput(w, position + glm::vec2{ 4, 4 }, glm::vec2{ size.x - 8, 8 }) -
               glm::vec2{ 4, 4 };

    {
        glm::vec2 max_size =
            glm::vec2(10000.0f); // glm::vec2(w->getSize()) - glm::vec2{ 0, UIRootMenu::getHeight() };

        glm::vec2 change_size     = { 0, 0 };
        glm::vec2 change_position = { 0, 0 };
        glm::vec2 grab_pos        = position + glm::vec2{ 0, 4 };
        glm::vec2 left_grab       = grabbables[1]->checkInput(w, grab_pos, glm::vec2{ 4, size.y - 8 });
        float change              = left_grab.x - grab_pos.x;
        change                    = size.x - glm::clamp(size.x - change, editor_min_width, max_size.x);
        change_size.x += change;
        change_position.x += change;

        grab_pos             = position + glm::vec2{ size.x - 4, 4 };
        glm::vec2 right_grab = grabbables[2]->checkInput(w, grab_pos, glm::vec2{ 4, size.y - 8 });
        change               = right_grab.x - grab_pos.x;
        change_size.x -= change;

        grab_pos           = position + glm::vec2{ 4, 0 };
        glm::vec2 top_grab = grabbables[3]->checkInput(w, grab_pos, glm::vec2{ size.x - 8, 4 });
        change             = top_grab.y - grab_pos.y;
        change             = size.y - glm::clamp(size.y - change, editor_min_width, max_size.y);
        change_size.y += change;
        change_position.y += change;

        grab_pos              = position + glm::vec2{ 4, size.y - 4 };
        glm::vec2 bottom_grab = grabbables[4]->checkInput(w, grab_pos, glm::vec2{ size.x - 8, 4 });
        change                = bottom_grab.y - grab_pos.y;
        change_size.y -= change;

        grab_pos          = position;
        glm::vec2 tl_grab = grabbables[5]->checkInput(w, grab_pos, glm::vec2{ 4, 4 });
        glm::vec2 change2 = tl_grab - grab_pos;
        change2 = size - glm::clamp(size - change2, { editor_min_width, editor_min_width }, max_size);
        change_size += change2;
        change_position += change2;

        grab_pos          = position + glm::vec2{ size.x - 4, 0 };
        glm::vec2 tr_grab = grabbables[6]->checkInput(w, grab_pos, glm::vec2{ 4, 4 });
        change2           = tr_grab - grab_pos;
        change2.y         = size.y - glm::clamp(size.y - change2.y, editor_min_width, max_size.y);
        change_size += change2 * glm::vec2{ -1, 1 };
        change_position.y += change2.y;

        grab_pos          = position + glm::vec2{ 0, size.y - 4 };
        glm::vec2 bl_grab = grabbables[7]->checkInput(w, grab_pos, glm::vec2{ 4, 4 });
        change2           = bl_grab - grab_pos;
        change2.x =
            size.x - glm::clamp(size.x - change2.x, editor_min_width, max_size.x);
        change_size += change2 * glm::vec2{ 1, -1 };
        change_position.x += change2.x;

        grab_pos          = position + glm::vec2{ size.x - 4, size.y - 4 };
        glm::vec2 br_grab = grabbables[8]->checkInput(w, grab_pos, glm::vec2{ 4, 4 });
        change2           = br_grab - grab_pos;
        change_size -= change2;

        position += change_position;
        size -= change_size;

        position = glm::clamp(position, { 16.0f - size.x, UIRootMenu::getHeight() },
            glm::vec2(w->getSize()) - 16.0f);
        size     = glm::clamp(size, { editor_min_width, editor_min_width }, glm::vec2(10000.0f));
    }
    // TODO: snapping?

    if (size != last_checked_size)
    {
        last_checked_size = size;
        updateLines();
        updateCursor();
    }
}

void UITextEditor::updateLines() {}

void UITextEditor::updateCursor() {}
