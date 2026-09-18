#include "raylib.h"
#include <stdio.h>
#include "raymath.h"
#include "rlgl.h"

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

typedef enum {
    NEW_SHAPE_SUBSTATE_PREVIEW,
    NEW_SHAPE_SUBSTATE_LINE_1,
    NEW_SHAPE_SUBSTATE_LINE_2,
} New_Shape_Substate;

static struct {
    Shape shape;
    Shape_Type preview_shape;
    Edit_Mode mode;
    New_Shape_Substate new_shape_substate;
    Vector2 line_start;
    Vector2 tri_second_vert;
} app = {0};

int main()
{
    const int width  = 800;
    const int height = 600;
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(width, height, "2D Shape Drawing App");

    while(!WindowShouldClose()) {
        if (IsKeyPressed(KEY_M)) {
            app.preview_shape = (app.preview_shape + 1)%SHAPE_COUNT;
        }

        Vector2 mouse_pos = GetMousePosition();

        if (app.mode == EDIT_MODE_NEW_SHAPE && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
                app.line_start = mouse_pos;
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_LINE_1;
            } else if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_1) {
                if (app.preview_shape == SHAPE_TRIANGLE)
                    app.new_shape_substate = NEW_SHAPE_SUBSTATE_LINE_2;
                else
                    app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
            } else if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_2) {
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
            }
        }

        Vector2 shape_icon = {mouse_pos.x + 25, mouse_pos.y - 25};
        Vector2 t0 = Vector2Add(shape_icon, (Vector2){0, 25});
        Vector2 t1 = Vector2Add(t0, (Vector2){30, 0});
        Vector2 t2 = Vector2Add(t0, (Vector2){15, -25});

        BeginDrawing();
            rlDisableBackfaceCulling();

            // draw partial line
            Color preview_color = BLUE;
            preview_color.a = 200;
            Color vert_color = RED;
            vert_color.a = 200;
            if (app.mode == EDIT_MODE_NEW_SHAPE && app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_1) {
                if (app.preview_shape != SHAPE_ELLIPSE) DrawLineV(app.line_start, mouse_pos, BLACK);

                DrawCircleV(app.line_start, 5, vert_color);
                DrawCircleV(mouse_pos, 5, vert_color);

                float radius = Vector2Length(Vector2Subtract(mouse_pos, app.line_start));
                Rectangle rect = {
                    .x = app.line_start.x,
                    .y = app.line_start.y,
                    .width  = mouse_pos.x - app.line_start.x,
                    .height = mouse_pos.y - app.line_start.y,
                };

                if (app.preview_shape == SHAPE_SQUARE) {
                    rect.x -= radius*0.5*sqrt(2);
                    rect.y -= radius*0.5*sqrt(2);
                    rect.width = radius*sqrt(2);
                    rect.height = radius*sqrt(2);
                }

                switch (app.preview_shape) {
                case SHAPE_SQUARE:
                    DrawRectangleRec(rect, preview_color);
                    DrawCircleLines(app.line_start.x, app.line_start.y, radius, BLACK);
                break;
                case SHAPE_RECTANGLE:
                    DrawRectangleRec(rect, preview_color);
                break;
                case SHAPE_CIRLCE:
                    DrawCircle(app.line_start.x, app.line_start.y, radius, preview_color);
                break;
                case SHAPE_ELLIPSE:
                    DrawEllipse(app.line_start.x, app.line_start.y, rect.width, rect.height, preview_color);
                    DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, BLACK);
                break;
                case SHAPE_TRIANGLE:
                    app.tri_second_vert = mouse_pos;
                break;
                default:
                }
            }

            if (app.mode == EDIT_MODE_NEW_SHAPE && app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_2) {
                DrawTriangle(app.line_start, app.tri_second_vert, mouse_pos, preview_color);
                DrawLineV(app.line_start, mouse_pos, BLACK);
                DrawLineV(app.line_start, app.tri_second_vert, BLACK);
                DrawLineV(app.tri_second_vert, mouse_pos, BLACK);
                DrawCircleV(app.line_start, 5, vert_color);
                DrawCircleV(app.tri_second_vert, 5, vert_color);
                DrawCircleV(mouse_pos, 5, vert_color);
            }

            // draw new shape icons next to mouse cursor
            if (app.mode == EDIT_MODE_NEW_SHAPE && app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
                switch (app.preview_shape) {
                case SHAPE_SQUARE:
                    DrawRectangle(shape_icon.x, shape_icon.y, 25, 25, BLUE);
                break;
                case SHAPE_RECTANGLE:
                    DrawRectangle(shape_icon.x, shape_icon.y, 50, 25, BLUE);
                break;
                case SHAPE_CIRLCE:
                    DrawCircle(shape_icon.x + 13, shape_icon.y + 13, 13, BLUE);
                break;
                case SHAPE_ELLIPSE:
                    DrawEllipse(shape_icon.x + 26, shape_icon.y + 13, 26, 13, BLUE);
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
