#include "raylib.h"
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 100

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Moving Pixels with Raylib");

    /* player */
    float player_pos_x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
    float player_padding = 100.0f;
    float player_pos_y = SCREEN_HEIGHT - PLAYER_HEIGHT - player_padding;
    float move_speed = 500.0f;

    /* projectile */
    float projectile_pos_x = player_pos_x;
    float projectile_pos_y = player_pos_y;
    float projectile_radius = 10.0f;
    float projectile_speed = 1000.0f;
    bool firing = false;

    /* enemy */
    float enemy_pos_x = SCREEN_WIDTH / 2;
    float enemy_pos_y = 0.0f;
    float enemy_radius = 50.0f;
    int enemy_health = 3;

    bool game_won = false;

    while(!WindowShouldClose()) {
        /* Input */
        float dt = GetFrameTime();
        if (IsKeyDown(KEY_A)) player_pos_x -= move_speed * dt;
        if (IsKeyDown(KEY_D)) player_pos_x += move_speed * dt;
        if (IsKeyPressed(KEY_SPACE)) firing = true;

        /* Move the projectile */
        if (firing) {
            projectile_pos_y -= projectile_speed * dt;

            if (projectile_pos_y < 0)  {
                firing = false;
                projectile_pos_x = player_pos_x;
                projectile_pos_y = player_pos_y;
            }
        } else {
            projectile_pos_x = player_pos_x;
        }

        /* update enemy */
        enemy_pos_x = (sin(GetTime()) * 0.5 + 0.5) * SCREEN_WIDTH;

        bool collided = CheckCollisionCircles(
            (Vector2) {projectile_pos_x, projectile_pos_y},
            projectile_radius,
            (Vector2) {enemy_pos_x, enemy_pos_y},
            enemy_radius
        );
        if (collided) {
            enemy_radius -= 10.0f;
            firing = false;
            projectile_pos_x = player_pos_x;
            projectile_pos_y = player_pos_y;

            enemy_health -= 1;
            if (enemy_health == 0)
                game_won = true;
        }

        /* Drawing */
        BeginDrawing();
            ClearBackground(DARKGREEN);
            if (!game_won) {
                DrawRectangle(player_pos_x, player_pos_y, PLAYER_WIDTH, PLAYER_HEIGHT, BLUE);
                if (firing)
                    DrawCircle(projectile_pos_x + PLAYER_WIDTH / 2, projectile_pos_y, projectile_radius, RED);
                DrawCircle(enemy_pos_x, enemy_pos_y, enemy_radius, MAROON);
            } else {
                const char * msg = "YOU WON!!!!";
                int width = MeasureText(msg, 50);
                DrawText(msg, SCREEN_WIDTH / 2 - width / 2, SCREEN_HEIGHT / 2, 50, BLACK);
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
