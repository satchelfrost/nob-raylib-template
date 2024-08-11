#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include "ext/nob.h"

#define WIDTH 1600
#define HEIGHT 900
#define WAYPOINT_RADIUS 10.0f
#define WAYPOINT_COLL_RADIUS WAYPOINT_RADIUS / 100.0f
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

Point waypoints[] = {
    {
        .pos = {
            .x = WIDTH / 4.0f,
            .y = (5.0 / 8.0) * HEIGHT,
        },
        .color = PURPLE,
    },
    {
        .pos = {
            .x = WIDTH / 2.0f,
            .y = HEIGHT / 2.0f,
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

#define NUM_WAYPOINTS NOB_ARRAY_LEN(waypoints)

typedef struct {
    Point *items;
    size_t count;
    size_t capacity;
} PlotPoints;

typedef enum {
    GOTO_PURPLE = 0,
    GOTO_GREEN,
    GOTO_BLUE,
} GotoState;

typedef struct {
    Vector2 pos;
    Color color;
    GotoState gotoState;
    bool oldestMode;
    Point waypoints[3];
    PlotPoints plotPoints;
} Drone;

Drone drones[] = {
    {
        .pos =  {
            .x = WIDTH / 4.0f,
            .y = (5.0 / 8.0) * HEIGHT,
        },
        .color = BLACK,
        .gotoState = GOTO_BLUE,
        .oldestMode = true,
    },
    {
        .pos =  {
            .x = WIDTH / 4.0f,
            .y = (5.0 / 8.0) * HEIGHT,
        },
        .color = RED,
        .gotoState = GOTO_GREEN,
        .oldestMode = false,
    },

};

#define NUM_DRONES NOB_ARRAY_LEN(drones)

int GetOldestPointIdx(Point *points, size_t numPoints)
{
    int idx = -1;
    float oldest = 0.0f;

    for (size_t i = 0; i < numPoints; i++) {
        if (points[i].time >= oldest) {
            oldest = points[i].time;
            idx = i;
        }
    }

    assert(idx != -1 && "this shouldn't happen");

    return idx;
}

float GetAveTime(Point *points, size_t numPoints)
{
    float sum = 0.0f;
    for (size_t i = 0; i < numPoints; i++)
        sum += points[i].time;

    if (numPoints) {
        float ave = sum / (float)numPoints;
        return ave;
    } else {
        return 0.0f;
    }
}


int main()
{
    /* Initialization */
    InitWindow(WIDTH, HEIGHT, "Next Oldest Time Heuristic");
    bool moving = true;
    float deltaTime = 0.0f;
    float plotTimer = 0.0f;
    bool flip = false; // used for next oldest time heuristic

    /* Initialize drone point state */
    for (size_t i = 0; i < NUM_DRONES; i++) {
        for (size_t j = 0; j < NUM_WAYPOINTS; j++) {
            drones[i].waypoints[j] = waypoints[j];
        }
    }

    /* Game loop */
    while(!WindowShouldClose()) {
        /* Input */
        if (IsKeyPressed(KEY_SPACE)) moving = !moving;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 pos = GetMousePosition();
            for (size_t i = 0; i < NUM_DRONES; i++)
                drones[i].waypoints[GOTO_GREEN].pos = pos;
        }

        /* Update drone position */
        if (moving) {
            deltaTime = GetFrameTime();
            for (size_t i = 0; i < NUM_DRONES; i++) {
                Vector2 dst = drones[i].waypoints[drones[i].gotoState].pos;
                Vector2 src = drones[i].pos;
                Vector2 dir = Vector2Subtract(dst, src);
                dir = Vector2Normalize(dir);
                Vector2 velocity = Vector2Scale(dir, deltaTime * SPEED);
                drones[i].pos = Vector2Add(src, velocity);
            }
        } else {
            deltaTime = 0.0f;
        }

        /* Plot update */
        plotTimer += deltaTime;
        if (plotTimer >= PLOT_INTERVAL) {
            plotTimer = 0.0f;
            for (size_t i = 0; i < NUM_DRONES; i++) {
                PlotPoints *plotPoints = &drones[i].plotPoints;
                if (plotPoints->count + 1 <= MAX_PLOT_POINTS) {
                    float av_time = GetAveTime(drones[i].waypoints, NUM_WAYPOINTS);
                    Point p = {
                        .pos = {
                            .x = PLOT_PADDING + plotPoints->count * STEP,
                            .y = HEIGHT - PLOT_PADDING - av_time * PLOT_HEIGHT * Y_SCALE_FACTOR,
                        },
                        .color = drones[i].color,
                        .time = av_time,
                    };

                    /* reset plot or update plot points */
                    if (p.pos.x >= WIDTH - PADDING) plotPoints->count = 0;
                    else nob_da_append(plotPoints, p);
                }
            }
        }

        /* Collision check */
        for (size_t i = 0; i < NUM_DRONES; i++) {
            for (size_t j = 0; j < NUM_WAYPOINTS; j++) {
                Drone *drone = &drones[i];
                Vector2 dst = drone->waypoints[j].pos;
                if (CheckCollisionCircles(drone->pos, DRONE_RADIUS, dst, WAYPOINT_COLL_RADIUS)) {
                    if (j == drone->gotoState) {
                        /* reset time */
                        drone->waypoints[j].time = 0.0f;
                        if (drone->oldestMode) {
                            /* oldest point mode */
                            drone->gotoState = GetOldestPointIdx(drone->waypoints, NUM_WAYPOINTS);
                        } else {
                            /* next oldest point (aka flip-flop) */
                            if (drone->gotoState == GOTO_PURPLE || drone->gotoState == GOTO_BLUE) {
                                drone->gotoState = GOTO_GREEN;
                            } else {
                                drone->gotoState = (flip) ? GOTO_PURPLE : GOTO_BLUE;
                                flip = !flip;
                            }
                        }
                    } else {
                        drone->waypoints[j].time += deltaTime;
                    }
                } else {
                    drone->waypoints[j].time += deltaTime;
                }
            }
        }

        /* Drawing */
        BeginDrawing();
            ClearBackground(RAYWHITE);
            /* Draw waypoints */
            for (size_t j = 0; j < NUM_WAYPOINTS; j++) {
                Point p1 = drones[0].waypoints[j];
                Point p2 = drones[1].waypoints[j];
                /* Only draw one set of waypoints */
                DrawCircleV(p1.pos, WAYPOINT_RADIUS, p1.color);

                const char *txt = TextFormat("%.1fs", p1.time); 
                DrawText(txt, p1.pos.x + 20.0f, p1.pos.y, FONT_SZ, drones[0].color);

                const char *txt2 = TextFormat("%.1fs", p2.time); 
                int width = MeasureText(txt2, FONT_SZ);
                DrawText(txt2, p2.pos.x - 20.0f - (float)width, p2.pos.y, FONT_SZ, drones[1].color);
            }

            /* Draw drones */
            for (size_t i = 0; i < NUM_DRONES; i++)
                DrawCircleV(drones[i].pos, DRONE_RADIUS, drones[i].color);

            /* Draw plot boundaries and text */
            DrawRectangleLines(PLOT_PADDING, HEIGHT - PLOT_PADDING - PLOT_HEIGHT + 1.0, WIDTH - 2.0f * PLOT_PADDING, PLOT_HEIGHT, BLACK);
            const char *title = TextFormat("Current Average Time");
            int txtWidth = MeasureText(title, 20);
            DrawText(title, WIDTH / 2.0f - txtWidth / 2.0f, HEIGHT - PLOT_PADDING - PLOT_HEIGHT - 20, 20.0f, BLACK);
            DrawText("0.0", PLOT_PADDING, HEIGHT - PLOT_PADDING, 20.0f, BLACK);
            DrawText("4.0", PLOT_PADDING, HEIGHT - PLOT_PADDING - PLOT_HEIGHT - 20, 20.0f, BLACK);

            /* Draw line plots */
            for (size_t i = 0; i < NUM_DRONES; i++) {
                PlotPoints pltPoints = drones[i].plotPoints;
                for (size_t j = 0; j < pltPoints.count; j++) {
                    if (j > 0) {
                        Vector2 p1 = pltPoints.items[j-1].pos;
                        Vector2 p2 = pltPoints.items[j].pos;
                        DrawLine(p1.x, p1.y, p2.x, p2.y, drones[i].color);
                        if (j == pltPoints.count - 1) {
                            float currAve = GetAveTime(drones[i].waypoints, NUM_WAYPOINTS);
                            if (i == 0) {
                                DrawText(TextFormat("%.1f", currAve), p2.x + 30.0f, p2.y, 20.0f, drones[i].color);
                            } else {
                                DrawText(TextFormat("%.1f", currAve), p2.x, p2.y, 20.0f, drones[i].color);
                            }
                        }
                    }
                }
            }

            /* Global average time */
            PlotPoints pltPoints = drones[0].plotPoints;
            float glblAve = GetAveTime(pltPoints.items, pltPoints.count);
            const char *txt = TextFormat("Global Average %.1fs (Oldest time mode)", glblAve); 
            DrawText(txt, 20, 20, FONT_SZ, BLACK);
            pltPoints = drones[1].plotPoints;
            glblAve = GetAveTime(pltPoints.items, pltPoints.count);
            const char *txt2 = TextFormat("Global Average %.1fs (Next oldest time heuristic)", glblAve); 
            DrawText(txt2, 20, 80, FONT_SZ, RED);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
