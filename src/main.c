#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

typedef struct {
    Vector2 origin;
    Vector2 direction;
} Ray_2D;

typedef struct {
    Vector2 a;
    Vector2 b;
} Segment_2D;

bool ray_segment_intersects(Ray_2D ray, Segment_2D segment, Vector2 *intersection)
{
    // source: https://rootllama.wordpress.com/2014/06/20/ray-line-segment-intersection-test-in-2d/
    Vector2 v1 = Vector2Subtract(ray.origin, segment.a);
    Vector2 v2 = Vector2Subtract(segment.b, segment.a);
    Vector2 v3 = (Vector2){-ray.direction.y, ray.direction.x};

    float t1 = (v2.x*v1.y - v2.y*v1.x)/Vector2DotProduct(v2, v3);
    float t2 = Vector2DotProduct(v1, v3)/Vector2DotProduct(v2, v3);

    if (!(t1 >= 0 && 0 <= t2 && t2 <= 1)) return false;

    Vector2 result = Vector2Add(ray.origin, Vector2Scale(ray.direction, t1));
    *intersection = result;

    return true;
}

int main()
{
    const int width  = 500;
    const int height = 500;
    InitWindow(width, height, "hello from raylib");

    int radius = 10;
    int padding = 50;
    Segment_2D segment = {
        .a = {padding+ radius, 250},
        .b = {500-padding-radius, 250},
    };
    Vector2 intersection = {0};
    Ray_2D ray = {0};

    while(!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            ray.origin = mouse;

        ray.direction = Vector2Subtract(mouse, ray.origin);
        bool intersects = ray_segment_intersects(ray, segment, &intersection);

        BeginDrawing();
            ClearBackground(WHITE);
            DrawCircleV(segment.a, radius, RED);
            DrawCircleV(segment.b, radius, RED);
            DrawLineV(ray.origin, GetMousePosition(), BLACK);
            DrawCircleV(ray.origin, radius, BLACK);
            if (intersects) DrawCircleV(intersection, radius, GOLD);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
