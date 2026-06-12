#include <glad.h>
#include <iostream>
#include <window.h>

using namespace AriaFlow;

int main()
{
    Window* w = new Window();

    while (!w->shouldClose())
    {
        w->poll();
        // TODO: process input
        // TODO: render UI
        w->makeCurrentContext();
        glViewport(0, 0, w->getSize().x, w->getSize().y);
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        w->present();
    }

    delete w;

    return 0;
}