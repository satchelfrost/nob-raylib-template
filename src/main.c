#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define FACTOR 60
#define SCREEN_WIDTH  (9 *  FACTOR)
#define SCREEN_HEIGHT (12 * FACTOR)
#define FONT_SIZE 40
#define FONT_VPADDING (FONT_SIZE / 2.0f)

#define SPEED 550
#define LR_SPEED 300
#define ACCELERATION (SPEED * 3)
#define DECELERATION (ACCELERATION * 2)
#define PLAYER_WIDTH 50.0f
#define GRAVITY 1000
#define EPSILON 0.01f
#define IDLE 1.0f

#define MIN_PLATFORM_WIDTH (PLAYER_WIDTH * 2)
#define MAX_PLATFORM_WIDTH (SCREEN_WIDTH / 4.0f)
#define PLATFORM_THICKNESS 10
#define NUM_PLATFORMS 5 
#define PLATFORM_SPACING (SCREEN_HEIGHT / NUM_PLATFORMS)
#define CLIMB_THRESH (PLAYER_WIDTH * 0.1f)
#define ANIM_ROT_SPEED 1000.0f

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))

typedef enum {
    PLAYER_STATE_CLIMB,
    PLAYER_STATE_SWING,
    PLAYER_STATE_FALL,
    PLAYER_STATE_DEAD,
} Player_State;

typedef enum {
    ANIM_STATE_NONE,
    ANIM_STATE_ROT,
} Animation_State;

typedef enum {
    GAME_STATE_INTRO,
    GAME_STATE_PLAYING,
    GAME_STATE_OVER,
} Game_State;

typedef struct {
    Vector2 pos;
    Vector2 velocity;
    Player_State state;

    Color hit_box_color; // debug
    Vector2 size;
    Rectangle hit_box;

    /* animation */
    Animation_State anim_state;
    Rectangle anim_rect;
    Color anim_color; // debug
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

Game_State update_player(Player *player, Platform *platforms)
{
    float dt = GetFrameTime();

    /* handle input */
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) {
        if (IsKeyDown(KEY_A) && IsKeyDown(KEY_D)) {
            if (fabsf(player->velocity.x) < IDLE) player->velocity.x = 0.0f;
            else if (player->velocity.x > 0.0) player->velocity.x -= DECELERATION * dt;
            else  player->velocity.x += DECELERATION * dt;
        } else if (IsKeyDown(KEY_A)) {
            float nx = ACCELERATION * dt;
            if (player->velocity.x - nx < -LR_SPEED) player->velocity.x = -LR_SPEED;
            else player->velocity.x -= nx;
        } else {
            float nx = ACCELERATION * dt;
            if (player->velocity.x + nx > LR_SPEED) player->velocity.x = LR_SPEED;
            else player->velocity.x += nx;
        }
    } else {
        if (fabsf(player->velocity.x) < IDLE) player->velocity.x = 0.0f;
        else if (player->velocity.x > 0.0) player->velocity.x -= DECELERATION * dt;
        else  player->velocity.x += DECELERATION * dt;
    }
    if (IsKeyPressed(KEY_W) && player->state == PLAYER_STATE_CLIMB) {
        player->velocity.y = -SPEED;
        player->state = PLAYER_STATE_SWING;
    }
    if (IsKeyPressed(KEY_S) && player->state == PLAYER_STATE_CLIMB) {
        player->state = PLAYER_STATE_FALL;
    }

    /* up-down translation */
    if (player->state != PLAYER_STATE_CLIMB) {
        player->velocity.y += GRAVITY * dt;

        float ny = player->pos.y + player->velocity.y * dt;
        if (ny + player->size.y / 2.0f > SCREEN_HEIGHT) {
            player->state = PLAYER_STATE_DEAD;
            player->pos.y = SCREEN_HEIGHT - player->size.y / 2.0f;
            player->velocity.y = 0.0f;
        }

        player->pos.y = ny;
    }

    /* left-right translation */
    player->pos.x += player->velocity.x * dt;
    if (player->pos.x - player->size.x / 2.0f < 0.0f) {
        player->pos.x = player->size.x / 2.0f;
        player->velocity.x = 0.0f;
    }
    if (player->pos.x + player->size.x / 2.0f > SCREEN_WIDTH) {
        player->pos.x = SCREEN_WIDTH - player->size.x / 2.0f;
        player->velocity.x = 0.0f;
    }

    /* collision */
    player->hit_box = (Rectangle) {
        .x = player->pos.x - PLAYER_WIDTH / 2.0f, // upper left corner x
        .y = player->pos.y - PLAYER_WIDTH / 2.0f, // upper left corner y
        .width  = player->size.x,
        .height = player->size.y,
    };
    bool colliding = false;
    for (size_t i = 0; i < NUM_PLATFORMS; i++) {
        Rectangle platform_rect = {
            .x = platforms[i].pos.x,
            .y = platforms[i].pos.y,
            .width = platforms[i].size.x,
            .height = platforms[i].size.y,
        };
        if (CheckCollisionRecs(player->hit_box, platform_rect) && player->pos.y <= platforms[i].pos.y) {
            colliding = true;
            platforms[i].color = PINK;
            if (player->state == PLAYER_STATE_FALL && platforms[i].pos.y - player->pos.y < CLIMB_THRESH) {
                player->pos.y = platforms[i].pos.y;
                player->velocity.y = 0.0f;
                player->state = PLAYER_STATE_CLIMB;
                player->anim_state = ANIM_STATE_NONE;
            }

            if (player->state == PLAYER_STATE_SWING) {
                platforms[i].color = GREEN;
                player->anim_state = ANIM_STATE_ROT;
            }
        } else {
            platforms[i].color = BLUE;
        }
    }

    if (!colliding && player->state != PLAYER_STATE_DEAD) player->state = PLAYER_STATE_FALL;

    /* update animation */
    player->anim_rect = (Rectangle) {
        .x = player->pos.x,
        .y = player->pos.y,
        .width = player->size.x,
        .height = player->size.y,
    };

    if (player->state == PLAYER_STATE_DEAD) return GAME_STATE_OVER;
    else return GAME_STATE_PLAYING;
}

void draw_game(const Player player, Platform *platforms)
{
    for (size_t i = 0; i < NUM_PLATFORMS; i++)
        DrawRectangleV(platforms[i].pos, platforms[i].size, platforms[i].color);
    Vector2 draw_pos = {player.pos.x - player.size.x / 2.0f, player.pos.y - player.size.y / 2.0f};
    DrawRectangleV(draw_pos, player.size, player.hit_box_color);

    if (player.anim_state == ANIM_STATE_ROT) {
        float rot_dir = (player.velocity.x >= 0.0f) ? 1.0f: -1.0f;
        DrawRectanglePro(player.anim_rect,
                         (Vector2){player.size.x / 2.0f, player.size.y / 2.0f},
                         GetTime() * ANIM_ROT_SPEED * rot_dir, player.anim_color);
    }
}

void draw_game_over()
{
    float canvas_height = 2.0f * FONT_SIZE + FONT_VPADDING;

    const char *msg1 = "Game Over";
    int txt_width = MeasureText(msg1, FONT_SIZE);
    DrawText(msg1, SCREEN_WIDTH / 2.0f - txt_width / 2.0f, SCREEN_HEIGHT / 2.0f - canvas_height / 2.0f, FONT_SIZE, BLACK);

    const char *msg2 = "Space to Try Again";
    txt_width = MeasureText(msg2, FONT_SIZE);
    DrawText(msg2,
             SCREEN_WIDTH / 2.0f - txt_width / 2.0f,
             SCREEN_HEIGHT / 2.0f - FONT_SIZE / 2.0f + canvas_height / 2.0f, FONT_SIZE, BLACK);
}

void draw_game_intro()
{
    float canvas_height = 2.0f * FONT_SIZE + FONT_VPADDING;

    const char *msg1 = "Movement - WASD";
    int txt_width = MeasureText(msg1, FONT_SIZE);
    DrawText(msg1, SCREEN_WIDTH / 2.0f - txt_width / 2.0f, SCREEN_HEIGHT / 2.0f - canvas_height / 2.0f, FONT_SIZE, BLACK);

    const char *msg2 = "Space to Start";
    txt_width = MeasureText(msg2, FONT_SIZE);
    DrawText(msg2,
             SCREEN_WIDTH / 2.0f - txt_width / 2.0f,
             SCREEN_HEIGHT / 2.0f - FONT_SIZE / 2.0f + canvas_height / 2.0f, FONT_SIZE, BLACK);
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "hello from raylib");

    srand(time(0));
    Game_State game_state = GAME_STATE_INTRO;
    Player_State last_state = PLAYER_STATE_FALL;

    /* initalize player and platforms */
    Player player = {
        .pos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT * 7.0f / 8.0f},
        .size = {PLAYER_WIDTH, PLAYER_WIDTH},
        .hit_box_color = RED,
        .state = PLAYER_STATE_FALL,
        .anim_color = BLUE,
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
    /* make sure the player lands on a platform to start the game */
    player.pos.x = platforms[0].size.x / 2.0f + platforms[0].pos.x;

    /* game loop */
    while(!WindowShouldClose()) {
        /* update */
        switch (game_state) {
        case GAME_STATE_PLAYING:
            game_state = update_player(&player, platforms);
            break;
        case GAME_STATE_OVER:
            if (IsKeyPressed(KEY_SPACE)) {
                game_state = GAME_STATE_PLAYING;
                player = (Player) {
                    .pos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT * 7.0f / 8.0f},
                    .size = {PLAYER_WIDTH, PLAYER_WIDTH},
                    .hit_box_color = RED,
                    .state = PLAYER_STATE_FALL,
                    .anim_color = BLUE,
                };
                player.pos.x = platforms[0].size.x / 2.0f + platforms[0].pos.x;
            }
            break;
        case GAME_STATE_INTRO:
            if (IsKeyPressed(KEY_SPACE)) game_state = GAME_STATE_PLAYING;
            break;
        default:
            break;
        }

        /* drawing */
        BeginDrawing();
            ClearBackground(RAYWHITE);
            switch (game_state) {
            case GAME_STATE_INTRO:
                draw_game_intro();
                break;
            case GAME_STATE_PLAYING:
                draw_game(player, platforms);
                break;
            case GAME_STATE_OVER:
                draw_game_over();
                break;
            default:
                break;
            }
        EndDrawing();

        /* state tracker for debugging */
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
