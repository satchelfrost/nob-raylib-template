#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include "ext/nob.h"

#define WIDTH  1600
#define HEIGHT 900
#define POINT_RADIUS 10.0f
#define DRONE_RADIUS 15.0f
#define PLOT_POINT_RADIUS 5.0f
#define SPEED 350
#define FONT_SZ 50
#define PADDING 50
#define PLOT_INTERVAL 0.2f
#define MAX_PLOT_POINTS 200 
#define STEP WIDTH / MAX_PLOT_POINTS
#define PLOT_HEIGHT 200.0f
#define PLOT_PADDING 20.0f 
#define Y_SCALE_FACTOR 0.25

typedef struct {
    Vector2 pos;
    float time;
    Color color;
} Point;

Point points[] = {
    {
        .pos = {
            .x = WIDTH / 4.0f,
            .y = (5.0 / 8.0) * HEIGHT,
        },
        .color = {
            .r = 255,
            .a = 255,
        },
    },
    {
        .pos = {
            .x = (5.0 / 8.0) * WIDTH,
            .y = (5.0 / 8.0) * HEIGHT,
        },
        .color = {
            .g = 255,
            .a = 255,
        },
    },
    {
        .pos = {
            .x = (5.0 / 8.0) * WIDTH,
            .y = HEIGHT / 4.0f,
        },
        .color = {
            .b = 255,
            .a = 255,
        },
    },
};

#define NUM_POINTS (int) (sizeof(points) / sizeof(points[0]))

typedef struct {
    Vector2 pos;
    Color color;
    Vector2 velocity;
} Drone;

Drone drone = {
    .pos =  {
        .x = WIDTH / 4.0f,
        .y = (5.0 / 8.0) * HEIGHT,
    },
    .color = {
        .r = 255,
        .g = 203,
        .b = 0,
        .a = 64, 
    },
};

typedef struct {
    Point *items;
    size_t count;
    size_t capacity;
} PlotPoints;

int GetOldestPointIdx()
{
    int idx = -1;
    float oldest = 0.0f;

    for (int i = 0; i < NUM_POINTS; i++) {
        if (points[i].time >= oldest) {
            oldest = points[i].time;
            idx = i;
        }
    }

    assert(idx != -1 && "this shouldn't happen");

    return idx;
}

float GetAveTime()
{
    float sum = 0.0f;
    for (int i = 0; i < NUM_POINTS; i++) {
        sum += points[i].time;
    }

    return sum / (float)NUM_POINTS;
}

typedef enum {
    GO_RED = 0,
    GO_GREEN,
    GO_BLUE,
} GoState;

int main()
{
    InitWindow(WIDTH, HEIGHT, "hello from raylib");

    bool moving = true;
    GoState goState = GO_GREEN;
    int nextPointIdx = goState;
    float deltaTime = 0.0f;
    float plotTimer = 0.0f;
    PlotPoints plotPoints = {0};
    bool getOldest = false;
    bool flip = false;

    while(!WindowShouldClose()) {
        /* Input */
        if (IsKeyPressed(KEY_SPACE)) moving = !moving;
        if (IsKeyPressed(KEY_M)) getOldest = !getOldest;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            points[1].pos = GetMousePosition();
        }

        /* Update drone position */
        if (moving) {
            deltaTime = GetFrameTime();
            Vector2 dir = Vector2Subtract(points[nextPointIdx].pos, drone.pos);
            dir = Vector2Normalize(dir);
            drone.velocity = Vector2Scale(dir, deltaTime * SPEED);
            drone.pos = Vector2Add(drone.pos, drone.velocity);
        } else {
            deltaTime = 0.0f;
        }

        /* Plot update */
        plotTimer += deltaTime;
        if (plotTimer >= PLOT_INTERVAL) {
            plotTimer = 0.0f;
            if (plotPoints.count + 1 <= MAX_PLOT_POINTS) {
                float av_time = GetAveTime();
                Point p = {
                    .pos = {
                        .x = PLOT_PADDING + plotPoints.count * STEP,
                        .y = HEIGHT - PLOT_PADDING - av_time * PLOT_HEIGHT * Y_SCALE_FACTOR,
                    },
                    .color = BLACK,
                };

                /* reset plot or update plot points */
                if (p.pos.x >= WIDTH - PADDING) plotPoints.count = 0;
                else nob_da_append(&plotPoints, p);
            }
        }

        /* Collision check */
        for (int i = 0; i < NUM_POINTS; i++) {
            if (getOldest) {
                if (CheckCollisionCircles(drone.pos, DRONE_RADIUS, points[i].pos, POINT_RADIUS / 100.0f)) {
                    points[i].time = 0.0f;
                    nextPointIdx = GetOldestPointIdx();
                } else {
                    points[i].time += deltaTime;
                }
            } else {
                if (CheckCollisionCircles(drone.pos, DRONE_RADIUS, points[goState].pos, POINT_RADIUS / 100.0f)) {
                    points[goState].time = 0.0f;
                    if (goState == GO_RED || goState == GO_BLUE) {
                        goState = GO_GREEN;
                        nextPointIdx = goState;
                    } else {
                        goState = (flip) ? GO_RED : GO_BLUE;
                        nextPointIdx = goState;
                        flip = !flip;
                    }
                } else {
                    points[i].time += deltaTime;
                }
            }
        }

        /* Drawing */
        BeginDrawing();
            ClearBackground(BLUE);
            for (int i = 0; i < NUM_POINTS; i++) {
                DrawCircleV(points[i].pos, POINT_RADIUS, points[i].color);
                DrawCircleV(drone.pos, DRONE_RADIUS, drone.color);

                const char *txt = TextFormat("%.1fs", points[i].time); 
                Vector2 pos = points[i].pos;
                DrawText(txt, pos.x, pos.y - POINT_RADIUS / 2.0 - FONT_SZ / 2.0 - PADDING, FONT_SZ, BLACK);
            }

            /* Draw plot boundaries and text */
            DrawRectangleLines(PLOT_PADDING, HEIGHT - PLOT_PADDING - PLOT_HEIGHT, WIDTH - 2.0f * PLOT_PADDING, PLOT_HEIGHT, BLACK);
            const char *title = TextFormat("Average time (plot interval=%.2fs)", PLOT_INTERVAL);
            int txtWidth = MeasureText(title, 20);
            DrawText(title, WIDTH / 2.0f - txtWidth / 2.0f, HEIGHT - PLOT_PADDING - PLOT_HEIGHT - 20, 20.0f, BLACK);
            DrawText("0.0", PLOT_PADDING, HEIGHT - PLOT_PADDING, 20.0f, BLACK);
            DrawText("4.0", PLOT_PADDING, HEIGHT - PLOT_PADDING - PLOT_HEIGHT - 20, 20.0f, BLACK);

            /* Draw line plot */
            for (size_t i = 0; i < plotPoints.count; i++) {
                if (i > 0) {
                    Vector2 p1 = plotPoints.items[i-1].pos;
                    Vector2 p2 = plotPoints.items[i].pos;
                    DrawLine(p1.x, p1.y, p2.x, p2.y, BLACK);
                }
            }

            const char *txt = TextFormat("Average time %.1fs", GetAveTime()); 
            DrawText(txt, 20, 20, FONT_SZ, BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
