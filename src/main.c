#include "raylib.h"
#include <stdio.h>
#include "raymath.h"

typedef enum {
    SHAPE_SQUARE,
    SHAPE_RECTANGLE,
    SHAPE_CIRLCE,
    SHAPE_ELLIPSE,
    SHAPE_TRIANGLE,
    SHAPE_COUNT,
} Shape_Type;

typedef struct {
    Shape_Type type;
    int radius0;
    int radius1;
    Rectangle rect;
    Vector2 p0, p1, p2;
} Shape;

typedef struct {
    Shape *items;
    size_t count; 
    size_t capacity; 
} Shapes;

typedef enum {
    EDIT_MODE_NEW_SHAPE,
    EDIT_MODE_SELECTED,
    EDIT_MODE_COUNT,
} Edit_Mode;

static struct {
    Shape shape;
    Shape_Type new_shape_ico;
    Edit_Mode mode;
} app = {0};

int main()
{
    const int width  = 800;
    const int height = 600;
    InitWindow(width, height, "2D Shape Drawing App");


    while(!WindowShouldClose()) {
        if (IsKeyPressed(KEY_M)) {
            printf("shape-%d\n", app.new_shape_ico);
            app.new_shape_ico = (app.new_shape_ico + 1)%SHAPE_COUNT;
        }

        Vector2 mouse_pos = GetMousePosition();
        Vector2 new_shape_ico = {mouse_pos.x + 25, mouse_pos.y - 25};
        Vector2 t0 = new_shape_ico;
        Vector2 t1 = Vector2Add(new_shape_ico, (Vector2){30, 0});
        Vector2 t2 = Vector2Add(new_shape_ico, (Vector2){15, -15});

        BeginDrawing();
            if (app.mode == EDIT_MODE_NEW_SHAPE) {
                switch (app.new_shape_ico) {
                case SHAPE_SQUARE:
                    DrawRectangle(new_shape_ico.x, new_shape_ico.y, 50, 50, RED);
                break;
                case SHAPE_RECTANGLE:
                    DrawRectangle(new_shape_ico.x, new_shape_ico.y, 100, 50, BLUE);
                break;
                case SHAPE_CIRLCE:
                    DrawCircle(new_shape_ico.x, new_shape_ico.y, 25, BLUE);
                break;
                case SHAPE_ELLIPSE:
                    DrawEllipse(new_shape_ico.x, new_shape_ico.y, 50, 25, BLUE);
                break;
                case SHAPE_TRIANGLE:
                    DrawTriangle(t0, t1, t2, BLUE);
                break;
                default:
                }
            }

            ClearBackground(WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
