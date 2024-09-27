#include "raylib.h"
#include <stdio.h>

int main()
{
    const int width  = 500;
    const int height = 500;
    InitWindow(width, height, "hello from raylib");

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLUE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
