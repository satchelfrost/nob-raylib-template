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
        .color = PURPLE,
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
        .color = GOLD,
    },
};

Point points2[3] = {0};

#define NUM_POINTS (int) (sizeof(points) / sizeof(points[0]))

typedef enum {
    GO_PURPLE = 0,
    GO_GREEN,
    GO_BLUE,
} GoState;

typedef struct {
    Vector2 pos;
    Color color;
    Vector2 velocity;
    GoState goState;
    int oldestIdx;
    bool oldestMode;
} Drone;

Drone drones[] = {
    {
        .pos =  {
            .x = WIDTH / 4.0f,
            .y = (5.0 / 8.0) * HEIGHT,
        },
        .color = BLACK,
        .oldestMode = true,
        .oldestIdx = 2,
    },
    {
        .pos =  {
            .x = WIDTH / 4.0f,
            .y = (5.0 / 8.0) * HEIGHT,
        },
        .color = RED,
        .goState = GO_GREEN,
        .oldestIdx = 1,
        .oldestMode = false,
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

float GetAveTime(bool firstSet)
{
    float sum = 0.0f;
    for (int i = 0; i < NUM_POINTS; i++) {
        if (firstSet)
            sum += points[i].time;
        else
            sum += points2[i].time;
    }

    return sum / (float)NUM_POINTS;
}

float GetGlblAve(PlotPoints points)
{
    float sum = 0.0f;
    for (int i = 0; i < (int)points.count; i++) {
        sum += points.items[i].time;
    }

    float ave = sum / (float)points.count;
    return ave;
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "hello from raylib");

    bool moving = true;
    float deltaTime = 0.0f;
    float plotTimer = 0.0f;
    PlotPoints plotPoints = {0};
    PlotPoints plotPoints2 = {0};
    bool flip = false;

    points2[0] = points[0];
    points2[1] = points[1];
    points2[2] = points[2];

    while(!WindowShouldClose()) {
        /* Input */
        if (IsKeyPressed(KEY_SPACE)) moving = !moving;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            points2[1].pos = points[1].pos = GetMousePosition();
        }

        /* Update drone position */
        if (moving) {
            for (int i = 0; i < (int)NOB_ARRAY_LEN(drones); i++) {
                deltaTime = GetFrameTime();
                Vector2 dir;
                if (drones[i].oldestMode) {
                    dir = Vector2Subtract(points[drones[i].oldestIdx].pos, drones[i].pos);
                } else {
                    dir = Vector2Subtract(points2[drones[i].oldestIdx].pos, drones[i].pos);
                }
                dir = Vector2Normalize(dir);
                drones[i].velocity = Vector2Scale(dir, deltaTime * SPEED);
                drones[i].pos = Vector2Add(drones[i].pos, drones[i].velocity);
            }
        } else {
            deltaTime = 0.0f;
        }

        /* Plot update */
        plotTimer += deltaTime;
        if (plotTimer >= PLOT_INTERVAL) {
            plotTimer = 0.0f;
            if (plotPoints.count + 1 <= MAX_PLOT_POINTS) {
                float av_time = GetAveTime(true);
                Point p = {
                    .pos = {
                        .x = PLOT_PADDING + plotPoints.count * STEP,
                        .y = HEIGHT - PLOT_PADDING - av_time * PLOT_HEIGHT * Y_SCALE_FACTOR,
                    },
                    .color = BLACK,
                    .time = av_time,
                };

                /* reset plot or update plot points */
                if (p.pos.x >= WIDTH - PADDING) plotPoints.count = 0;
                else nob_da_append(&plotPoints, p);

                av_time = GetAveTime(false);
                Point p2 = {
                    .pos = {
                        .x = PLOT_PADDING + plotPoints2.count * STEP,
                        .y = HEIGHT - PLOT_PADDING - av_time * PLOT_HEIGHT * Y_SCALE_FACTOR,
                    },
                    .color = BLACK,
                    .time = av_time,
                };
                if (p2.pos.x >= WIDTH - PADDING) plotPoints2.count = 0;
                else nob_da_append(&plotPoints2, p2);
            }
        }

        /* Collision check */
        for (int i = 0; i < NUM_POINTS; i++) {
            for (int j = 0; j < (int)NOB_ARRAY_LEN(drones); j++) {
                if (drones[j].oldestMode) {
                    if (CheckCollisionCircles(drones[j].pos, DRONE_RADIUS, points[i].pos, POINT_RADIUS / 100.0f)) {
                        points[i].time = 0.0f;
                        drones[j].oldestIdx = GetOldestPointIdx();
                    } else {
                        points[i].time += deltaTime;
                    }
                } else {
                    if (CheckCollisionCircles(drones[j].pos, DRONE_RADIUS, points2[drones[j].goState].pos, POINT_RADIUS / 100.0f)) {
                        points2[drones[j].goState].time = 0.0f;
                        if (drones[j].goState == GO_PURPLE || drones[j].goState == GO_BLUE) {
                            drones[j].goState = GO_GREEN;
                            drones[j].oldestIdx= GO_GREEN;
                        } else {
                            drones[j].goState = (flip) ? GO_PURPLE : GO_BLUE;
                            drones[j].oldestIdx = drones[j].goState;
                            flip = !flip;
                        }
                    } else {
                        points2[i].time += deltaTime;
                    }
                }
            }
        }

        /* Drawing */
        BeginDrawing();
            ClearBackground(BLUE);
            for (int i = 0; i < NUM_POINTS; i++) {
                DrawCircleV(points[i].pos, POINT_RADIUS, points[i].color);

                const char *txt = TextFormat("%.1fs", points[i].time); 
                Vector2 pos = points[i].pos;
                DrawText(txt, pos.x + 20, pos.y, FONT_SZ, BLACK);

                const char *txt2 = TextFormat("%.1fs", points2[i].time); 
                int width = MeasureText(txt2, FONT_SZ);
                pos = points2[i].pos;
                DrawText(txt2, pos.x - 20 - width, pos.y, FONT_SZ, RED);
            }

            for (int i = 0; i < (int)NOB_ARRAY_LEN(drones); i++) 
                DrawCircleV(drones[i].pos, DRONE_RADIUS, drones[i].color);

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
            for (size_t i = 0; i < plotPoints2.count; i++) {
                if (i > 0) {
                    Vector2 p1 = plotPoints2.items[i-1].pos;
                    Vector2 p2 = plotPoints2.items[i].pos;
                    DrawLine(p1.x, p1.y, p2.x, p2.y, RED);
                }
            }

            const char *txt = TextFormat("curr ave. %.1fs, glbl av. %.1fs (Oldest time mode)", GetAveTime(true), GetGlblAve(plotPoints)); 
            DrawText(txt, 20, 20, FONT_SZ, BLACK);
            const char *txt2 = TextFormat("curr av. %.1fs, glbl av. %.1fs (flip flop mode)", GetAveTime(false), GetGlblAve(plotPoints2)); 
            DrawText(txt2, 20, 80, FONT_SZ, RED);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
