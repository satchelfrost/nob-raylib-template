#include "raylib.h"
#include <math.h>

int main()
{
    int screen_width = 800;
    int screen_height = 800;
    InitWindow(screen_width, screen_height, "Moving Pixels");

    /* player */
    float player_width = 50.0f;
    float player_height = 100.0f;
    float player_pos_x = screen_width / 2 - player_width / 2;
    float padding = 50.0f;
    float player_pos_y = screen_height - player_height - padding;
    float player_speed = 500.0f;

    /* enemy */
    float enemy_pos_x = screen_width / 2;
    float enemy_pos_y = 0.0f;
    float enemy_radius = 100.0f;
    int enemy_health = 3;

    /* bullet */
    float bullet_pos_x = player_pos_x + player_width / 2;
    float bullet_pos_y = player_pos_y;
    float bullet_radius = 10.0f;
    float bullet_speed = 1000.0f;
    bool firing = false;

    bool game_won = false;

    while (!WindowShouldClose()) {
        /* Input */
        float dt = GetFrameTime();
        if (IsKeyDown(KEY_D)) player_pos_x = player_pos_x + player_speed * dt;
        if (IsKeyDown(KEY_A)) player_pos_x = player_pos_x - player_speed * dt;
        if (IsKeyPressed(KEY_SPACE)) firing = true;

        /* Move enemy */
        enemy_pos_x = (sin(GetTime()) * 0.5 + 0.5) * screen_width;

        /* Move bullet */
        if (firing) {
            bullet_pos_y = bullet_pos_y - bullet_speed * dt;

            if (bullet_pos_y < 0) {
                firing = false;
                bullet_pos_x = player_pos_x + player_width / 2;
                bullet_pos_y = player_pos_y;
            }
        } else {
            bullet_pos_x = player_pos_x + player_width / 2;
        }

        bool collided = CheckCollisionCircles(
            (Vector2) {bullet_pos_x, bullet_pos_y},
            bullet_radius,
            (Vector2) {enemy_pos_x, enemy_pos_y},
            enemy_radius
        );

        if (collided) {
            // deal damage
            enemy_radius = enemy_radius - 20.0f;
            enemy_health = enemy_health - 1;

            // reset bullet
            firing = false;
            bullet_pos_x = player_pos_x + player_width / 2;
            bullet_pos_y = player_pos_y;

            // Did we win?
            if (enemy_health == 0)
                game_won = true;
        }

        /* Drawing */
        BeginDrawing();
            ClearBackground(BLUE);
            if (game_won) {
                const char *msg = "You Won!!!";
                int width = MeasureText(msg, 50);
                DrawText(msg, screen_width / 2 - width / 2, screen_height / 2, 50, BLACK);
            } else {
                DrawRectangle(player_pos_x, player_pos_y, player_width, player_height, DARKPURPLE);
                DrawCircle(enemy_pos_x, enemy_pos_y, enemy_radius, ORANGE);
                if (firing) 
                    DrawCircle(bullet_pos_x, bullet_pos_y, bullet_radius, RED);
            }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
