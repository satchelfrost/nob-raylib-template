#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <raymath.h>

#define FACTOR 100
#define WINDOW_WIDTH  (16 * FACTOR)
#define WINDOW_HEIGHT ( 9 * FACTOR)
#define MOVE_SPEED 1.0f
#define DISC_THICKNESS 0.0f
#define MIN_ACCEPTABLE_COST 0.0f
#define LEARNING_RATE 0.01f
#define EPS 0.01f

#define POINT_COUNT 30
static Vector3 points[POINT_COUNT] = {0};

typedef struct {
    Vector3 point;
    Vector3 normal;
} Plane;

float point_height_from_plane(Plane plane, Vector3 point)
{
    return fabsf(Vector3DotProduct(plane.normal, Vector3Subtract(plane.point, point)) /
                 Vector3DotProduct(plane.normal, plane.normal));
}

float cost(Plane plane)
{
    float best_sum = 0;
    for (size_t i = 0; i < POINT_COUNT; i++)
        best_sum += point_height_from_plane(plane, points[i]);
    return best_sum / POINT_COUNT;
}

int main()
{
    srand(time(0));
    Camera camera = {
        .position   = {2.0f, 2.0f, 4.0f},
        .target     = {0.0f, 0.0f, 0.0f},
        .up         = {0.0f, 1.0f, 0.0f},
        .fovy       = 45.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Normal Estimation Via Gradient Descent");

    float thickness = DISC_THICKNESS;
    Vector3 cube_pos = {0.0f, 0.0f, 0.0f};
    float cube_dim = 2.0;
    Plane plane = {.normal = {0.0f, 1.0f, 0.0f}};
    size_t steps = 0;
    Plane guessed_plane = {
        .normal = {
            rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX,
        },
    };
    guessed_plane.normal = Vector3Normalize(guessed_plane.normal);

    for (size_t i = 0; i < POINT_COUNT; i++) {
        float rand_radius  = rand() / (float)RAND_MAX;
        float rand_theta   = rand() / (float)RAND_MAX;
        float rand_height  = rand() / (float)RAND_MAX;
        points[i].x = rand_radius * cosf(rand_theta * 2.0f * M_PI) * cube_dim * 0.5;
        points[i].y = (rand_height * 2.0f - 1.0f) * DISC_THICKNESS;
        points[i].z = rand_radius * sinf(rand_theta * 2.0f * M_PI) * cube_dim * 0.5;
    }

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (IsKeyDown(KEY_A)) camera.position.x -= MOVE_SPEED * dt;
        if (IsKeyDown(KEY_W)) camera.position.y += MOVE_SPEED * dt;
        if (IsKeyDown(KEY_S)) camera.position.y -= MOVE_SPEED * dt;
        if (IsKeyDown(KEY_D)) camera.position.x += MOVE_SPEED * dt;

        if (IsKeyPressed(KEY_T)) {
            thickness += IsKeyDown(KEY_LEFT_SHIFT) ? (-dt) : (dt);
            for (size_t i = 0; i < POINT_COUNT; i++) {
                float rand_height = rand() / (float)RAND_MAX;
                points[i].y = (rand_height * 2.0f - 1.0f) * thickness;
            }
        }

        if (IsKeyDown(KEY_U)) {
            float c = cost(guessed_plane);
            if (c > MIN_ACCEPTABLE_COST) {
                Plane px = {.normal = Vector3Add(guessed_plane.normal, (Vector3){EPS, 0.0f, 0.0f})};
                Plane py = {.normal = Vector3Add(guessed_plane.normal, (Vector3){0.0f, EPS, 0.0f})};
                Plane pz = {.normal = Vector3Add(guessed_plane.normal, (Vector3){0.0f, 0.0f, EPS})};
                guessed_plane.normal.x = guessed_plane.normal.x - LEARNING_RATE * (cost(px) - c) / EPS;
                guessed_plane.normal.y = guessed_plane.normal.y - LEARNING_RATE * (cost(py) - c) / EPS;
                guessed_plane.normal.z = guessed_plane.normal.z - LEARNING_RATE * (cost(pz) - c) / EPS;
                guessed_plane.normal = Vector3Normalize(guessed_plane.normal);
                steps++;
            }
        }

        if (IsKeyPressed(KEY_L)) {
            printf("Average height (exact normal) = %f\n", cost(plane));
            printf("Average height (guessed normal) = %f\n", cost(guessed_plane));
            printf("Steps until converged %zu\n", steps);
            steps = 0;
        }

        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))  {
            float dir = (IsKeyDown(KEY_LEFT)) ? -1.0f : 1.0f;
            for (size_t i = 0; i < POINT_COUNT; i++)
                points[i] = Vector3RotateByAxisAngle(points[i], (Vector3){0.0f, 1.0f, 0.0f}, dir * dt);
            plane.normal = Vector3RotateByAxisAngle(plane.normal, (Vector3){0.0f, 1.0f, 0.0f}, dir * dt);
        }
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN))  {
            float dir = (IsKeyDown(KEY_UP)) ? -1.0f : 1.0f;
            for (size_t i = 0; i < POINT_COUNT; i++)
                points[i] = Vector3RotateByAxisAngle(points[i], (Vector3){1.0f, 0.0f, 0.0f}, dir * dt);
            plane.normal = Vector3RotateByAxisAngle(plane.normal, (Vector3){1.0f, 0.0f, 0.0f}, dir * dt);
        }

        BeginDrawing();
            ClearBackground(BLUE);
            BeginMode3D(camera);
                DrawCubeWires(cube_pos, cube_dim, cube_dim, cube_dim, BLACK);
                for (size_t i = 0; i < POINT_COUNT; i++)
                    DrawSphere(points[i], 0.02, RED);
                DrawLine3D(plane.point, plane.normal, GREEN);
                DrawLine3D(guessed_plane.point, guessed_plane.normal, RED);
            EndMode3D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
