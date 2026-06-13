#include "user_interface.h"
#include "window.h"

#include <glad.h>
#include <iostream>

using namespace AriaFlow;

int main()
{
    Window* w = new Window();
    w->makeCurrentContext();
    UIRenderer* r = new UIRenderer();

    r->clear();
    r->addText({ 0, 0 }, 0.0f, {}, "Hello, World!", { 1, 1, 1 });
    r->addNineSlice({ 36 * 0, 64 }, 0.0f, { 36, 36 }, 0, { 1, 1, 1, 1 });
    r->addNineSlice({ 36 * 1, 64 }, 0.0f, { 36, 36 }, 1, { 1, 1, 1, 1 });
    r->addNineSlice({ 36 * 2, 64 }, 0.0f, { 36, 36 }, 2, { 1, 1, 1, 1 });
    r->addNineSlice({ 36 * 2, 64 }, 0.0f, { 36, 36 }, 3, { 1, 1, 1, 1 });
    float x = 0;
    for (int i = 0; i < 16; ++i, x += 12)
    {
        r->addSimple({ x, 128 }, 0.0f, { 12, 12 }, i, { 0, 0 }, { 1, 1 });
    }
    r->finalise();

    while (!w->shouldClose())
    {
        // TODO: process input
        // TODO: render UI

        //r->draw(w);
        w->makeCurrentContext();
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        r->draw(w);
        w->present();
        w->poll();
    }

    delete w;

    return 0;
}