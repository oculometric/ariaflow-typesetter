#pragma once

namespace BBUI
{

class Style
{
    // TODO: this!
};

class Transform
{
    // TODO: this!
};

class Component
{
    // TODO: acts as a base class for generic components, providing base behaviour
    // provides position, size
    // provides default ui element behaviour and characteristics
    // overridable functions:
    // mouse down, mouse up, mouse clicked, mouse enter, mouse exit, mouse moved, mouse dragged
    // key down, key up, key repeat
    // arrange children
    // get minimum/maximum size
    // everything has to be private - changes to the component means calls to the renderer
    // optionally provides hover tooltips
};

class Canvas
{
    // TODO: contains and manages ui elements
    // manages style
    // manages the renderer
    // it draws those components, you just draw the canvas
    // it also delivers input events to components
    // it also calls arrange on its components
    // these functions are ONLY called when needed:
    // input events only delivered to components when they happen AND they should actually receive them
    // arrange only called when the canvas size changes
    // manages which element is currently highlighted/focused, navigation using tab/shift-tab
    // allows configuration of window menu, right-click menu
};

} // namespace BBUI