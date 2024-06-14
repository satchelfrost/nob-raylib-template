#include "raylib.h"
#include <math.h>
#include "ext/nob.h"

typedef struct {
    float x;
    float y;
    float radius;
    bool active;
} Projectile;

Projectile projectiles[3] = {0};

void init_projectiles(float x, float y)
{
    for (size_t i = 0; i < NOB_ARRAY_LEN(projectiles); i++) {
        Projectile p = {
            .x  = x,
            .y  = y,
            .radius = 10.0f,
            .active = false,
        };
        projectiles[i] = p;
    }
}

int main()
{
    int screen_width  = 500;
    int screen_height = 600;
    InitWindow(screen_width, screen_height, "hello from raylib");

    /* rectangle params */
    int rect_width  = 50;
    int rect_height = 100;
    float pos_x = screen_width / 2 - rect_width / 2;
    float pos_y = screen_height / 2 - rect_height / 2 + 100.0f;
    float move_speed = 500.0f;

    init_projectiles(pos_x + rect_width / 2, pos_y);

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
        /* input */
        float dt = GetFrameTime();
        if (IsKeyDown(KEY_D)) pos_x += move_speed * dt;
        if (IsKeyDown(KEY_A)) pos_x -= move_speed * dt;
        if (IsKeyDown(KEY_S)) pos_y += move_speed * dt;
        if (IsKeyDown(KEY_W)) pos_y -= move_speed * dt;

        if (IsKeyPressed(KEY_SPACE)) {
            for (size_t i = 0; i < NOB_ARRAY_LEN(projectiles); i++) {
                if (!projectiles[i].active) {
                    projectiles[i].active = true;
                    break;
                }
            }
        }

        /* firing sequence*/
        for (size_t i = 0; i < NOB_ARRAY_LEN(projectiles); i++) {
            if (projectiles[i].active) {
                if (projectiles[i].y > 0)
                    projectiles[i].y -= move_speed * dt * 1.5;
                else
                    projectiles[i].active = false;
            }
        }

        /* Collision check */
        Vector2 target_pos = {target_pos_x, target_pos_y};
        for (size_t i = 0; i < NOB_ARRAY_LEN(projectiles); i++) {
            Vector2 projectile_pos = {projectiles[i].x, projectiles[i].y};
            if (projectiles[i].active) {
                if (CheckCollisionCircles(projectile_pos, projectiles[i].radius, target_pos, target_radius)) {
                    target_curr_color = target_hit_color;
                    hit = true;
                    projectiles[i].active = false;
                }
            }
        }

        for (size_t i = 0; i < NOB_ARRAY_LEN(projectiles); i++) {
            if (!projectiles[i].active) {
                projectiles[i].x = pos_x + rect_width / 2;
                projectiles[i].y = pos_y;
            }
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
            for (size_t i = 0; i < NOB_ARRAY_LEN(projectiles); i++) {
                Projectile *p = &projectiles[i];
                if (p->active) DrawCircle(p->x, p->y, p->radius, RED);
            }

            /*draw character*/
            DrawRectangle(pos_x, pos_y, rect_width, rect_height, BLUE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
