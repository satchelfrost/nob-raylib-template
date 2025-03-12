#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH  500
#define SCREEN_HEIGHT 500
#define CIRCLE_RADIUS 10.0f
#define CIRCLE_ROW_COUNT 20 
#define SPACING_X (SCREEN_WIDTH  / CIRCLE_ROW_COUNT) 
#define SPACING_Y (SCREEN_HEIGHT / CIRCLE_ROW_COUNT) 

Vector2 screen_to_norm(Vector2 screen)
{
    return (Vector2){
        .x = screen.x / SCREEN_WIDTH,
        .y = screen.y / SCREEN_WIDTH,
    };
}

Vector2 norm_to_math(Vector2 norm)
{
    return (Vector2){
        .x = norm.x * CIRCLE_ROW_COUNT - CIRCLE_ROW_COUNT / 2.0f,
        .y = norm.y * CIRCLE_ROW_COUNT - CIRCLE_ROW_COUNT / 2.0f,
    };
}

Vector2 math_to_norm(Vector2 math)
{
    return (Vector2){
        .x = (math.x + CIRCLE_ROW_COUNT * 0.5) / CIRCLE_ROW_COUNT,
        .y = (math.y + CIRCLE_ROW_COUNT * 0.5) / CIRCLE_ROW_COUNT,
    };
}

Vector2 norm_to_screen(Vector2 norm)
{
    return (Vector2){
        .x = norm.x * SCREEN_WIDTH,
        .y = norm.y * SCREEN_HEIGHT,
    };
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "hello from raylib");

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);

            /* draw circles in the shape of a square */
            for (int i = 0; i < CIRCLE_ROW_COUNT; i++) {
                for (int j = 0; j < CIRCLE_ROW_COUNT; j++) {
                    Vector2 pos = {
                        .x = i * SPACING_X + SPACING_X / 2.0f,
                        .y = j * SPACING_Y + SPACING_Y / 2.0f,
                    };
                    DrawCircleV(pos, CIRCLE_RADIUS, RED);
                }
            }

            /* sample those same circles but in the shape of a larger circle */
            for (int i = 0; i < CIRCLE_ROW_COUNT; i++) {
                for (int j = 0; j < CIRCLE_ROW_COUNT; j++) {
                    Vector2 pos = {
                        .x = i * SPACING_X + SPACING_X / 2.0f,
                        .y = j * SPACING_Y + SPACING_Y / 2.0f,
                    };
                    Vector2 norm = screen_to_norm(pos);
                    Vector2 math = norm_to_math(norm);
                    float r, phi;
                    if (math.x * math.x > math.y * math.y) {
                        r = math.x;
                        phi = (M_PI * math.y) / (4.0f * math.x);
                    } else {
                        r = math.y;
                        phi = (M_PI / 2.0f) - (M_PI / 4.0f) * (math.x / math.y);
                    }
                    Vector2 new_pos = {
                        .x = r * cosf(phi),
                        .y = r * sinf(phi) 
                    };
                    Vector2 norm_math = math_to_norm(new_pos);
                    Vector2 final_pos = norm_to_screen(norm_math);
                    DrawCircleV(final_pos, CIRCLE_RADIUS, BLUE);
                }
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
