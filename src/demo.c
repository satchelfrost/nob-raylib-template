#include "raylib.h"
#include <math.h>

int main()
{
    int screen_width  = 500;
    int screen_height = 600;
    InitWindow(screen_width, screen_height, "hello from raylib");

    /*#region get rect son*/
    int rect_width  = 50;
    int rect_height = 100;
    float pos_x = screen_width / 2 - rect_width / 2;
    float pos_y = screen_height / 2 - rect_height / 2 + 100.0f;
    /*#endregion don't get rect son*/

    float move_speed = 500.0f;
    float circle_pos_x = pos_x + rect_width / 2;
    float circle_pos_y = pos_y;
    float projectile_radius = 10.0f;
    bool firing = false;

    /* Target */
    float target_radius = 100.0f;
    float target_pos_x = screen_width / 2;
    float target_pos_y = 0;
    Color target_color = MAROON;
    Color target_curr_color = target_color;
    Color target_hit_color = GOLD;
    float timer = 0.0f;
    bool hit = false;

    SetTargetFPS(60);
    while(!WindowShouldClose()) {
        /* move rectangle */
        float dt = GetFrameTime();
        if (IsKeyDown(KEY_D)) pos_x += move_speed * dt;
        if (IsKeyDown(KEY_A)) pos_x -= move_speed * dt;
        if (IsKeyDown(KEY_S)) pos_y += move_speed * dt;
        if (IsKeyDown(KEY_W)) pos_y -= move_speed * dt;
        if (IsKeyPressed(KEY_SPACE)) firing = true;

        /* firing sequence*/
        if (firing) {
            if (circle_pos_y > 0) {
                circle_pos_y -= move_speed * dt * 1.5;
            } else {
                firing = false;
                circle_pos_x = pos_x + rect_width / 2;
                circle_pos_y = pos_y;
            }
        }

        /* Collision check */
        Vector2 projectile_pos = {circle_pos_x, circle_pos_y};
        Vector2 target_pos     = {target_pos_x, target_pos_y};
        if (CheckCollisionCircles(projectile_pos, projectile_radius, target_pos, target_radius)) {
            target_curr_color = target_hit_color;
            firing = false;
            hit = true;
        }

        if (hit) {
            timer += dt;
            if (timer > 0.2f) {
                target_curr_color = target_color;
                hit = false;
                timer = 0.0f;
            }
        }

        /* move target */
        target_pos_x = (sin(GetTime()) * 0.5f + 0.5f) * screen_width;

        BeginDrawing();
            ClearBackground(WHITE);
            /*draw target*/
            DrawCircle(target_pos_x, target_pos_y, target_radius, target_curr_color);

            /*draw projectile*/
            if (firing)
                DrawCircle(circle_pos_x, circle_pos_y, projectile_radius, RED);
            else {
                circle_pos_x = pos_x + rect_width / 2;
                circle_pos_y = pos_y;
            }

            /*draw character*/
            DrawRectangle(pos_x, pos_y, rect_width, rect_height, BLUE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
