#include "raylib.h"
#include <stdio.h>
#include "raymath.h"
#include "rlgl.h"

#include "../nob.h"

// #define DEBUG_BOUNDING_BOX

#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

typedef enum {
    SHAPE_SQUARE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_ELLIPSE,
    SHAPE_TRIANGLE,
    SHAPE_COUNT,
} Shape_Type;

typedef struct {
    Shape_Type type;
    int radius0;
    int radius1;
    Vector2 position;
    Rectangle rect;
    Rectangle bounds;
    Color color;
    Vector2 p0, p1, p2;
    bool clicked;
    bool selected;
} Shape;

typedef struct {
    Shape *items;
    size_t count; 
    size_t capacity; 
} Shapes;

typedef enum {
    EDIT_MODE_NEW_SHAPE,
    EDIT_MODE_SELECT, // select mode substate
    EDIT_MODE_COUNT,
} Edit_Mode;

typedef enum {
    NEW_SHAPE_SUBSTATE_PREVIEW,
    NEW_SHAPE_SUBSTATE_LINE_1,
    NEW_SHAPE_SUBSTATE_LINE_2,
} New_Shape_Substate;

typedef enum {
    BUTTON_SELECT,
    BUTTON_NEW_SHAPE,
    BUTTON_PALETTE,
    BUTTON_COUNT,
} Button;

static struct {
    Shapes shapes;
    Shape_Type preview_shape;
    Color preivew_color;
    Edit_Mode mode;
    New_Shape_Substate new_shape_substate;
    Vector2 line_start;      // initial mouse click
    Vector2 tri_second_vert; // secondary mouse click (specificallly only for triangles)
    size_t selected_shape_index;
    Texture translate_widget_texture;
    bool skip_preview;

    struct {
        Color color;
        int width, height;
        int btn_height;
    } panel;
} app = {0};

void configure_ui()
{
    app.panel.color      = GRAY;
    app.panel.width      = 100;
    app.panel.height     = GetScreenHeight();
    app.panel.btn_height = 100;
    app.translate_widget_texture = LoadTexture("assets/translate_32x32.png");
}

void draw_side_panel()
{
    DrawRectangle(0, 0, app.panel.width, app.panel.height, app.panel.color);
}

Shape create_shape(Shape_Type type)
{
    Vector2 mouse_pos = GetMousePosition();
    float radius = Vector2Length(Vector2Subtract(mouse_pos, app.line_start));
    Rectangle rect = {
        .x = app.line_start.x,
        .y = app.line_start.y,
        .width  = mouse_pos.x - app.line_start.x,
        .height = mouse_pos.y - app.line_start.y,
    };
    Shape shape = {
        .type = type,
        .color = PURPLE,
    };
    switch (type) {
    case SHAPE_SQUARE: {
        rect.x -= radius*0.5*sqrt(2);
        rect.y -= radius*0.5*sqrt(2);
        rect.width = radius*sqrt(2);
        rect.height = radius*sqrt(2);
        shape.rect = rect;
        shape.bounds = rect;
    } break;
    case SHAPE_RECTANGLE: {
        shape.rect = rect;
        shape.bounds = rect;
        if (shape.bounds.width < 0) {
            shape.bounds.x += shape.bounds.width;
            shape.bounds.width *= -1.0;
        }
        if (shape.bounds.height < 0) {
            shape.bounds.y += shape.bounds.height;
            shape.bounds.height *= -1.0;
        }
    } break;
    case SHAPE_CIRCLE: {
        shape.radius0 = radius;
        shape.position = app.line_start;
        shape.bounds = (Rectangle) {
            .x = app.line_start.x - radius,
            .y = app.line_start.y - radius,
            .width  = 2*radius,
            .height = 2*radius,
        };
    } break;
    case SHAPE_ELLIPSE: {
        shape.radius0 = rect.width;
        shape.radius1 = rect.height;
        shape.position = app.line_start;
        shape.bounds = (Rectangle) {
            .x = app.line_start.x - rect.width,
            .y = app.line_start.y - rect.height,
            .width  = 2*rect.width,
            .height = 2*rect.height,
        };
        if (shape.bounds.width < 0) {
            shape.bounds.x += shape.bounds.width;
            shape.bounds.width *= -1.0;
        }
        if (shape.bounds.height < 0) {
            shape.bounds.y += shape.bounds.height;
            shape.bounds.height *= -1.0;
        }
    } break;
    case SHAPE_TRIANGLE: {
        shape.p0 = app.line_start;
        shape.p1 = app.tri_second_vert;
        shape.p2 = mouse_pos;
        float min_x = MIN(MIN(shape.p0.x, shape.p1.x), shape.p2.x);
        float min_y = MIN(MIN(shape.p0.y, shape.p1.y), shape.p2.y);
        float max_x = MAX(MAX(shape.p0.x, shape.p1.x), shape.p2.x);
        float max_y = MAX(MAX(shape.p0.y, shape.p1.y), shape.p2.y);
        shape.bounds = (Rectangle){
            .x = min_x,
                .y = min_y,
                .width  = max_x - min_x + 1,
                .height = max_y - min_y + 1,
        };
    } break;
    default: UNREACHABLE("shape unrecognized");
    }

    return shape;
}

void draw_shapes()
{
    for (size_t i = 0; i < app.shapes.count; i++) {
        Shape shape = app.shapes.items[i];
        switch (shape.type) {
        case SHAPE_SQUARE:
            DrawRectangleRec(shape.rect, shape.color);
        break;
        case SHAPE_RECTANGLE:
            DrawRectangleRec(shape.rect, shape.color);
        break;
        case SHAPE_CIRCLE:
            DrawCircle(shape.position.x, shape.position.y, shape.radius0, shape.color);
        break;
        case SHAPE_ELLIPSE:
            DrawEllipse(shape.position.x, shape.position.y, shape.radius0, shape.radius1, shape.color);
        break;
        case SHAPE_TRIANGLE:
            DrawTriangle(shape.p0, shape.p1, shape.p2, shape.color);
        break;
        default:
        }
    }

    for (size_t i = 0; i < app.shapes.count; i++) {
        Shape shape = app.shapes.items[i];
        if (shape.clicked) {
            DrawRectangleLines(shape.bounds.x, shape.bounds.y, shape.bounds.width, shape.bounds.height,
                               shape.selected ? YELLOW : BLACK);
        }
    }

#ifdef DEBUG_BOUNDING_BOX
    for (size_t i = 0; i < app.shapes.count; i++) {
        Shape shape = app.shapes.items[i];
        DrawRectangleLines(shape.bounds.x, shape.bounds.y, shape.bounds.width, shape.bounds.height, BLACK);
    }
#endif // DEBUG_BOUNDING_BOX
}

void draw_shape_preview()
{
    if (app.mode != EDIT_MODE_NEW_SHAPE) return;

    Color preview_color = BLUE;
    preview_color.a = 200;
    Vector2 mouse_pos = GetMousePosition();

    if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_1) {
        if (app.preview_shape == SHAPE_TRIANGLE) DrawLineV(app.line_start, mouse_pos, BLACK);

        DrawCircleV(app.line_start, 5, RED);

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
        break;
        case SHAPE_RECTANGLE:
            DrawRectangleRec(rect, preview_color);
        break;
        case SHAPE_CIRCLE:
            DrawCircle(app.line_start.x, app.line_start.y, radius, preview_color);
        break;
        case SHAPE_ELLIPSE:
            DrawEllipse(app.line_start.x, app.line_start.y, rect.width, rect.height, preview_color);
        break;
        case SHAPE_TRIANGLE:
            app.tri_second_vert = mouse_pos;
        break;
        default:
        }
    } else if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_2) {
        DrawTriangle(app.line_start, app.tri_second_vert, mouse_pos, preview_color);
        DrawLineV(app.line_start, mouse_pos, BLACK);
        DrawLineV(app.line_start, app.tri_second_vert, BLACK);
        DrawLineV(app.tri_second_vert, mouse_pos, BLACK);
        DrawCircleV(app.line_start, 5, RED);
        DrawCircleV(app.tri_second_vert, 5, RED);
    }
}

void draw_shape_preview_icon()
{
    Vector2 mouse_pos = GetMousePosition();
    Vector2 shape_icon = {mouse_pos.x + 25, mouse_pos.y - 25};
    Vector2 t0 = Vector2Add(shape_icon, (Vector2){0, 25});
    Vector2 t1 = Vector2Add(t0, (Vector2){30, 0});
    Vector2 t2 = Vector2Add(t0, (Vector2){15, -25});

    if (app.mode == EDIT_MODE_NEW_SHAPE && app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
        switch (app.preview_shape) {
        case SHAPE_SQUARE:
            DrawRectangle(shape_icon.x, shape_icon.y, 25, 25, BLUE);
        break;
        case SHAPE_RECTANGLE:
            DrawRectangle(shape_icon.x, shape_icon.y, 50, 25, BLUE);
        break;
        case SHAPE_CIRCLE:
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
}

bool is_button_clicked(Button btn)
{
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return false;

    Vector2 point = GetMousePosition();
    Rectangle rect = {
        .x = 0,
        .y = btn*app.panel.btn_height,
        .width = app.panel.width,
        .height = app.panel.btn_height,
    };

    return CheckCollisionPointRec(point, rect);
}

void draw_selected_shape_tr_widget()
{
    Shape shape = {0};
    bool selected_shape_found = false;
    for (size_t i = 0; i < app.shapes.count; i++) {
        shape = app.shapes.items[i];
        if (shape.selected) {
            selected_shape_found = true;
            break;
        }
    }

    if (!selected_shape_found) return;

    int x = shape.bounds.x + shape.bounds.width/2  - app.translate_widget_texture.width/2;
    int y = shape.bounds.y + shape.bounds.height/2 - app.translate_widget_texture.height/2;
    DrawTexture(app.translate_widget_texture, x, y, WHITE);
    DrawCircle(shape.bounds.x, shape.bounds.y, 5, BLACK);
    DrawCircle(shape.bounds.x+shape.bounds.width, shape.bounds.y, 5, BLACK);
    DrawCircle(shape.bounds.x+shape.bounds.width, shape.bounds.y+shape.bounds.height, 5, BLACK);
    DrawCircle(shape.bounds.x, shape.bounds.y+shape.bounds.height, 5, BLACK);
}

int main()
{
    const int width  = 800;
    const int height = 600;
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(width, height, "2D Shape Drawing App");
    configure_ui();

    struct {
        Shape **items;
        size_t count;
        size_t capacity;
    } clicked_shapes = {0};

    while(!WindowShouldClose()) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0  && app.mode == EDIT_MODE_NEW_SHAPE &&
            app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
            if (wheel < 0) app.preview_shape = (app.preview_shape + SHAPE_COUNT - 1)%SHAPE_COUNT;
            else           app.preview_shape = (app.preview_shape + 1)%SHAPE_COUNT;
        }

        Vector2 mouse_pos = GetMousePosition();

        /* handle select mode */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
            bool selected_one = false;
            clicked_shapes.count = 0;
            for (size_t i = app.shapes.count; i > 0; i--) {
                Shape *shape = &app.shapes.items[i-1];
                if ((shape->clicked = CheckCollisionPointRec(mouse_pos, shape->bounds))) {
                    da_append(&clicked_shapes, shape);
                    if (!selected_one) {
                        shape->selected = true;
                        selected_one = true;
                    } else {
                        shape->selected = false;
                    }
                } else {
                    shape->selected = false;
                }
            }
            Edit_Mode prev_mode = app.mode;
            app.mode = (selected_one) ? EDIT_MODE_SELECT : EDIT_MODE_NEW_SHAPE;
            if (selected_one) app.selected_shape_index = 0;

            /* if we just changed de selcted, then skip drawing the preview shape this frame */
            if (app.mode == EDIT_MODE_NEW_SHAPE && prev_mode == EDIT_MODE_SELECT) {
                app.skip_preview = true;
            }
        }

        if (wheel != 0.0  && app.mode == EDIT_MODE_SELECT) {
            if (clicked_shapes.count) {
                size_t count = clicked_shapes.count;
                if (wheel < 0) app.selected_shape_index = (app.selected_shape_index + count - 1)%count;
                else           app.selected_shape_index = (app.selected_shape_index + 1)%count;
                for (size_t i = 0; i < clicked_shapes.count; i++) {
                    Shape *shape = clicked_shapes.items[i];
                    shape->selected = app.selected_shape_index == i;
                }
            }
        }

        // if (app.mode == EDIT_MODE_SELECT && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        //     for (size_t i = 0; i < app.shapes.count; i++) {
        //         Shape *shape = &app.shapes.items[i];
        //         if (shape->selected) {
        //             Rectangle translate_widget_bb = {
        //                 .x = shape->bounds.x + shape->bounds.width/2  - app.translate_widget_texture.width/2,
        //                 .y = shape->bounds.y + shape->bounds.height/2 - app.translate_widget_texture.height/2,
        //                 .width  = app.translate_widget_texture.width,
        //                 .height = app.translate_widget_texture.height,
        //             };
        //             if (CheckCollisionPointRec(mouse_pos, translate_widget_bb)) {
        //                 shape->rect.x = mouse_pos.x - shape->rect.width/2;
        //                 shape->rect.y = mouse_pos.y - shape->rect.height/2;
        //                 shape->bounds = shape->rect;
        //             }
        //         }
        //     }
        // }

        // reset or cancel current shape draw
        if (app.mode == EDIT_MODE_NEW_SHAPE && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
        }

        if (app.mode == EDIT_MODE_NEW_SHAPE && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !app.skip_preview) {
            if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
                app.line_start = mouse_pos;
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_LINE_1;
            } else if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_1) {
                if (app.preview_shape == SHAPE_TRIANGLE) {
                    app.new_shape_substate = NEW_SHAPE_SUBSTATE_LINE_2;
                } else {
                    app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
                    da_append(&app.shapes, create_shape(app.preview_shape));
                    // printf("new shape added %d\n", app.preview_shape);
                }
            } else if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_2) {
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
                da_append(&app.shapes, create_shape(app.preview_shape));
                // printf("new shape added %d\n", app.preview_shape);
            }
        }

        if (is_button_clicked(BUTTON_SELECT))
            printf("select button\n");


        BeginDrawing(); {
            ClearBackground(WHITE);
            rlDisableBackfaceCulling();

            draw_shapes();

            if (app.mode == EDIT_MODE_SELECT)
                draw_selected_shape_tr_widget();

            if (app.mode == EDIT_MODE_NEW_SHAPE && !app.skip_preview) {
                draw_shape_preview();
                draw_shape_preview_icon();
            }

            draw_side_panel();
        } EndDrawing();

        app.skip_preview = false;
    }
    CloseWindow();
    return 0;
}
