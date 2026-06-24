#pragma once

#include "bbui/canvas.h"

namespace BBUI
{

class Button : Component
{
    // TODO: clickable button element, can have text, icon
    // callback event on click
    // enter if focused
};

class Label : Component
{
    // TODO: simple label, can have text, icon
};

class Panel : Component
{
    // TODO: contains other components, arranges them, has a background
};

class Menu : Component
{
    // TODO: a vertical auto-arranging component, always on top
    // can contain buttons, labels, dividers, and submenus
    // callbacks for each button
};

class ArrangeBox : Component
{
    // TODO: subdivides its space and arranges its children according to a grid pattern of rows/columns
};

class Slider : Component
{
    // TODO: allows the user to drag select within a range
    // callback events on change, slide release
};

class Selector : Component
{
    // TODO: allows the user to pick from a selection
    // callback event on change
};

class RadioButton : Component
{
    // TODO: allows the user to pick from several vertically arranged clickable labels
    // callback event on change
};

class CheckButton : Component
{
    // TODO: allows the user to check/uncheck a clickable label
    // callback event on change
};

class WindowBox : Component
{
    // TODO: provides a title and a drawable area
    // optionally can be moved and/or resized
    // optionally can have icon-only buttons top left/right
    // callbacks for each button, moved, move release
};

class DialogBox : Component
{
    // TODO: provides one, two, or three buttons, a title, an icon, and a message block
    // callbacks for each button
    // button types (so escape, enter can be bound automatically)
};

class TextBox : Component
{
    // TODO: provides single line text editing functionality
    // callback event on change, enter
    // optional restrictions for numbers, floats
};

class TextArea : Component
{
    // TODO: provides large scale multiline editing
    // callback on change
};

}