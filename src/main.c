#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define RADIUS 10
#define NUM_CIRCLES 4

Vector2 positions[NUM_CIRCLES] = {0};
Vector2 local_origin = {500.0f, 200.0f};
Vector2 local_right  = {500.0, 0.0};
Vector2 local_down   = {0.0, 500.0};

void calc_positions()
{
    positions[0] = local_origin;
    positions[1] = Vector2Add(local_origin, local_right);
    positions[2] = Vector2Add(local_origin, local_down);
    positions[3] = Vector2Add(Vector2Add(local_origin, local_right), local_down);
}

void rotate_deg(float deg)
{
    local_right = Vector2Rotate(local_right, deg * PI / 180.0f);
    local_down.x = -local_right.y;
    local_down.y =  local_right.x;
    calc_positions();
}

int main()
{
    const int width  = 1600;
    const int height = 900;
    InitWindow(width, height, "Normalized coordinate system");

    /* calculate initial positions based on vectors */
    calc_positions();

    while(!WindowShouldClose()) {
        if (IsKeyPressed(KEY_UP))   rotate_deg(-20.0f);
        if (IsKeyPressed(KEY_DOWN)) rotate_deg( 20.0f);

        /* calculate coordinates */
        Vector2 mouse_pos = GetMousePosition();
        Vector2 p = Vector2Subtract(mouse_pos, local_origin);

        // noralized x
        Vector2 d1_norm = Vector2Normalize(local_right);
        float d1_mag  = Vector2Length(local_right);
        float d1_scale = Vector2DotProduct(p, local_right) / d1_mag;
        float numerator_1 = Vector2Length(Vector2Scale(d1_norm, d1_scale));
        float x = numerator_1 / d1_mag; 

        // noralized y
        Vector2 d2_norm = Vector2Normalize(local_down);
        float d2_mag  = Vector2Length(local_down);
        float d2_scale = Vector2DotProduct(p, local_down) / d2_mag;
        float numerator_2 = Vector2Length(Vector2Scale(d2_norm, d2_scale));
        float y = numerator_2 / d2_mag; 

        /* drawing section */
        BeginDrawing();
            ClearBackground(BLUE);

            DrawText(TextFormat("(x, y) = (%.2f, %.2f)", x, y), 20, 20, 50, BLACK);
            DrawText(TextFormat("(x, y) = (%.2f, %.2f)", x * 2.0f - 1.0, y * 2.0 - 1.0), 20, 70, 50, BLACK);
            DrawText("Rotate - Key Up/Down", 20, 120, 40, BLACK);

            /* draw line and cursor only if mous is inside of box */
            if (x > 0.0 && x < 1.0 && y > 0.0 && y < 1.0 && d1_scale > 0.0 && d2_scale > 0.0) {
                DrawLineV(local_origin, mouse_pos, BLACK);
                DrawCircleV(mouse_pos, RADIUS, PINK);
            }

            /* Draw my coordinate system */
            for (int i = 0; i < NUM_CIRCLES; i++)
                DrawCircleV(positions[i], RADIUS, RED);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
