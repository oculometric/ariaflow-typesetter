#include "user_interface.h"
#include "window.h"

using namespace AriaFlow;

bool AriaFlow::insideRect(glm::vec2 point, glm::vec2 top_left, glm::vec2 size)
{
    if (point.x < top_left.x) return false;
    if (point.y < top_left.y) return false;
    if (point.x >= top_left.x + size.x) return false;
    if (point.y >= top_left.y + size.y) return false;

    return true;
}

void AriaFlow::trackWindowResizeFixedSize(std::shared_ptr<Window> w, glm::vec2& top_left, glm::vec2 size)
{
    glm::vec2 old_window_size = w->getLastSize();
    glm::vec2 new_window_size = w->getSize();

    glm::vec2 midpoint     = top_left + (size / 2.0f);
    glm::vec2 bottom_right = top_left + size;

    if ((glm::abs(midpoint.x - (old_window_size.x / 2.0f)) < (old_window_size.x / 8.0f)) &&
        !(top_left.x < 100.0f || bottom_right.x > old_window_size.x - 100.0f))
        top_left.x += (new_window_size - old_window_size).x / 2.0f;
    else if (midpoint.x > old_window_size.x / 2.0f)
        top_left.x += (new_window_size - old_window_size).x;

    if ((glm::abs(midpoint.y - (old_window_size.y / 2.0f)) < (old_window_size.y / 8.0f)) &&
        !(top_left.y < 100.0f || bottom_right.y > old_window_size.y - 100.0f))
        top_left.y += (new_window_size - old_window_size).y / 2.0f;
    else if (midpoint.y > old_window_size.y / 2.0f)
        top_left.y += (new_window_size - old_window_size).y;
}

void AriaFlow::trackWindowResizeScaleSize(std::shared_ptr<Window> w, glm::vec2& top_left, glm::vec2& size)
{
    glm::vec2 old_window_size = w->getLastSize();
    glm::vec2 new_window_size = w->getSize();

    glm::vec2 bottom_right = top_left + size;

    auto scaleRelative = [old_window_size, new_window_size](glm::vec2 point) -> glm::vec2
    {
        bool left   = point.x < old_window_size.x / 2.0f;
        bool center = glm::abs(point.x - (old_window_size.x / 2.0f)) < (old_window_size.x / 6.0f);
        bool top    = point.y < old_window_size.y / 2.0f;
        bool middle = glm::abs(point.y - (old_window_size.y / 2.0f)) < (old_window_size.y / 6.0f);
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
    size         = bottom_right - top_left;
}
