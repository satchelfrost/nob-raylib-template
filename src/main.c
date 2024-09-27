#include "raylib.h"
#include <math.h>

#define FACTOR 60
#define SCREEN_WIDTH  (9 *  FACTOR)
#define SCREEN_HEIGHT (12 * FACTOR)
#define SPEED 550
#define ACCELERATION (SPEED * 3)
#define DECELERATION (ACCELERATION * 2)
#define GRAVITY 1000
#define EPSILON 0.0001f
#define IDLE 0.001f

typedef struct {
    Vector2 pos;
    Vector2 velocity;
    Vector2 size;
    Color color;
    bool jumping;
} Player;

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "hello from raylib");

    Player player = {
        .pos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT * 7.0f / 8.0f},
        .size = {50.0f, 50.0f},
        .color = RED,
    };

    while(!WindowShouldClose()) {
        /* update */
        float dt = GetFrameTime();
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) {
            if (IsKeyDown(KEY_A) && IsKeyDown(KEY_D)) {
                if (fabsf(player.velocity.x - IDLE) < EPSILON) player.velocity.x = 0.0f;
                else if (player.velocity.x > 0.0) player.velocity.x -= DECELERATION * dt;
                else  player.velocity.x += DECELERATION * dt;
            } else if (IsKeyDown(KEY_A)) {
                float nx = ACCELERATION * dt;
                if (player.velocity.x - nx < -SPEED) player.velocity.x = -SPEED;
                else player.velocity.x -= nx;
            } else {
                float nx = ACCELERATION * dt;
                if (player.velocity.x + nx > SPEED) player.velocity.x = SPEED;
                else player.velocity.x += nx;
            }

        } else {
            if (fabsf(player.velocity.x - IDLE) < EPSILON) player.velocity.x = 0.0f;
            else if (player.velocity.x > 0.0) player.velocity.x -= DECELERATION * dt;
            else  player.velocity.x += DECELERATION * dt;
        }

        if (IsKeyPressed(KEY_W) && !player.jumping) {
            player.velocity.y = -SPEED;
            player.jumping = true;
        }

        /* physics */
        /* up-down translation */
        if (player.jumping) player.velocity.y += GRAVITY * dt;
        float ny = player.pos.y + player.velocity.y * dt;
        if (ny + player.size.y / 2.0f > SCREEN_HEIGHT && player.jumping) {
            player.jumping = false;
            player.pos.y = SCREEN_HEIGHT - player.size.y / 2.0f - 10.0f;
            player.velocity.y = 0.0f;
        } else {
            player.pos.y = ny;
        }

        /* left-right translation */
        player.pos.x += player.velocity.x * dt;
        if (player.pos.x - player.size.x / 2.0f < 0.0f) {
            player.pos.x = player.size.x / 2.0f;
            player.velocity.x = 0.0f;
        }
        if (player.pos.x + player.size.x / 2.0f > SCREEN_WIDTH) {
            player.pos.x = SCREEN_WIDTH - player.size.x / 2.0f;
            player.velocity.x = 0.0f;
        }

        /* drawing */
        BeginDrawing();
            ClearBackground(RAYWHITE);
            Vector2 draw_pos = {player.pos.x - player.size.x / 2.0f, player.pos.y - player.size.y / 2.0f};
            DrawRectangleV(draw_pos, player.size, player.color);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
