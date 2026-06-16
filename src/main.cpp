#include "application.h"

int main()
{
    // r->addText({ 0, 0 }, 0.0f, {}, "Hello, World!", { 1, 1, 1 });
    // r->addNineSlice({ 36 * 0, 64 }, 0.0f, { 36, 36 }, 0, { 1, 1, 1, 1 });
    // r->addNineSlice({ 36 * 1, 64 }, 0.0f, { 36, 36 }, 1, { 1, 1, 1, 1 });
    // r->addNineSlice({ 36 * 2, 64 }, 0.0f, { 36, 36 }, 2, { 1, 1, 1, 1 });
    // r->addNineSlice({ 36 * 2, 64 }, 0.0f, { 36, 36 }, 3, { 1, 1, 1, 1 });
    // float x = 0;
    // for (int i = 0; i < 16; ++i, x += 12)
    // {
    //     r->addSimple({ x, 128 }, 0.0f, { 12, 12 }, i, { 0, 0 }, { 1, 1 });
    // }
    AriaFlow::Application app;

    app.run();

    return EXIT_SUCCESS;
}