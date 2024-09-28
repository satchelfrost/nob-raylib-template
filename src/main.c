#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define FACTOR 60
#define SCREEN_WIDTH  (9 *  FACTOR)
#define SCREEN_HEIGHT (12 * FACTOR)

#define SPEED 550
#define LR_SPEED 300
#define ACCELERATION (SPEED * 3)
#define DECELERATION (ACCELERATION * 2)
#define PLAYER_WIDTH 50.0f
#define GRAVITY 1000
#define EPSILON 0.01f
#define IDLE 0.001f

#define MIN_PLATFORM_WIDTH (PLAYER_WIDTH * 2)
// #define MAX_PLATFORM_WIDTH (SCREEN_WIDTH / 2.0f)
#define MAX_PLATFORM_WIDTH (SCREEN_WIDTH / 4.0f)
#define PLATFORM_THICKNESS 10
#define NUM_PLATFORMS 5 
#define PLATFORM_SPACING (SCREEN_HEIGHT / NUM_PLATFORMS)

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))

typedef enum {
    PLAYER_STATE_CLIMB,
    PLAYER_STATE_SWING,
    PLAYER_STATE_FALL,
    PLAYER_STATE_DEAD,
} Player_State;


typedef struct {
    Vector2 pos;
    Vector2 velocity;
    Vector2 size;
    Color color;
    Player_State state;
} Player;

typedef struct {
    Vector2 pos;
    Vector2 size;
    Color color;
} Platform;

float rand_float()
{
    return (float)rand() / RAND_MAX;
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "hello from raylib");

    srand(time(0));
    Player_State last_state = PLAYER_STATE_DEAD;
    Player player = {
        .pos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT * 7.0f / 8.0f},
        .size = {PLAYER_WIDTH, PLAYER_WIDTH},
        .color = RED,
        .state = PLAYER_STATE_FALL,
    };
    Platform platforms[NUM_PLATFORMS] = {0};
    for (size_t i = 0; i < NUM_PLATFORMS; i++) {
        platforms[i].pos.x = SCREEN_WIDTH * rand_float();
        platforms[i].pos.y = player.pos.y + player.size.y / 2.0f - i * PLATFORM_SPACING;
        platforms[i].size.x = MIN_PLATFORM_WIDTH + rand_float() * (MAX_PLATFORM_WIDTH - MIN_PLATFORM_WIDTH);
        platforms[i].size.y = PLATFORM_THICKNESS;
        platforms[i].color = BLUE;

        float total_width = platforms[i].pos.x + platforms[i].size.x;
        if (total_width > SCREEN_WIDTH) {
            platforms[i].pos.x = total_width - SCREEN_WIDTH;
        }
    }
    player.pos.x = platforms[0].size.x / 2.0f + platforms[0].pos.x;

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
                if (player.velocity.x - nx < -LR_SPEED) player.velocity.x = -LR_SPEED;
                else player.velocity.x -= nx;
            } else {
                float nx = ACCELERATION * dt;
                if (player.velocity.x + nx > LR_SPEED) player.velocity.x = LR_SPEED;
                else player.velocity.x += nx;
            }

        } else {
            if (fabsf(player.velocity.x - IDLE) < EPSILON) player.velocity.x = 0.0f;
            else if (player.velocity.x > 0.0) player.velocity.x -= DECELERATION * dt;
            else  player.velocity.x += DECELERATION * dt;
        }

        if (IsKeyPressed(KEY_W) && player.state == PLAYER_STATE_CLIMB) {
            player.velocity.y = -SPEED;
            player.state = PLAYER_STATE_SWING;
        }
        if (IsKeyPressed(KEY_S) && player.state == PLAYER_STATE_CLIMB) {
            player.state = PLAYER_STATE_FALL;
        }

        /* up-down translation */
        if (player.state != PLAYER_STATE_CLIMB)
            player.velocity.y += GRAVITY * dt;
        float ny = player.pos.y + player.velocity.y * dt;
        if (ny + player.size.y / 2.0f > SCREEN_HEIGHT) {
            player.state = PLAYER_STATE_DEAD;
            player.pos.y = SCREEN_HEIGHT - player.size.y / 2.0f;
            player.velocity.y = 0.0f;
        } else if (player.state != PLAYER_STATE_CLIMB){
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

        /* collision */
        Rectangle player_rect = {
            .x = player.pos.x - PLAYER_WIDTH / 2.0f, // upper left corner x
            .y = player.pos.y - PLAYER_WIDTH / 2.0f, // upper left corner y
            .width  = player.size.x,
            .height = player.size.y,
        };
        bool collided = false;
        for (size_t i = 0; i < NUM_PLATFORMS; i++) {
            Rectangle platform_rect = {
                .x = platforms[i].pos.x,
                .y = platforms[i].pos.y,
                .width = platforms[i].size.x,
                .height = platforms[i].size.y,
            };
            if (CheckCollisionRecs(player_rect, platform_rect) && player.pos.y <= platforms[i].pos.y) {
                collided = true;
                platforms[i].color = PINK;
                if (player.state == PLAYER_STATE_FALL) {
                    player.pos.y = platforms[i].pos.y;
                    player.velocity.y = 0.0f;
                    player.state = PLAYER_STATE_CLIMB;
                }
            } else {
                platforms[i].color = BLUE;
            }
        }

        if (!collided && player.state != PLAYER_STATE_DEAD) {
            player.state = PLAYER_STATE_FALL;
        }

        /* drawing */
        BeginDrawing();
            ClearBackground(RAYWHITE);
            Vector2 draw_pos = {player.pos.x - player.size.x / 2.0f, player.pos.y - player.size.y / 2.0f};
            for (size_t i = 0; i < NUM_PLATFORMS; i++)
                DrawRectangleV(platforms[i].pos, platforms[i].size, platforms[i].color);
            DrawRectangleV(draw_pos, player.size, player.color);
        EndDrawing();

        /* state tracker */
        if (player.state != last_state) {
            TraceLog(LOG_INFO, "%s --> %s",
                     (const char *[]){"climb", "swing", "fall", "dead"}[last_state],
                     (const char *[]){"climb", "swing", "fall", "dead"}[player.state]);
            last_state = player.state;
        }
    }

    CloseWindow();
    return 0;
}
