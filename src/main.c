#include "raylib.h"
#include <stdio.h>
#include "raymath.h"
#include "rlgl.h"
#include <time.h>

#include "stb_image_write.h"

#define NOB_IMPLEMENTATION
#include "../nob.h"

// #define SHOW_DEBUG_BOUNDING_BOXES
#define MIN_WIDTH  20
#define MIN_HEIGHT 20

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
    Color color;
    Vector2 p0, p1, p2;
    bool clicked;
    bool highlighted;
    bool selected_translate;
    bool selected_top_left;
    bool selected_top_right;
    bool selected_bottom_left;
    bool selected_bottom_right;
    RenderTexture2D target;
} Shape;

typedef struct {
    Shape *items;
    size_t count; 
    size_t capacity; 
} Shapes;

typedef struct {
    Shape_Type type;
    int radius0;
    int radius1;
    Vector2 position;
    Rectangle rect;
    Color color;
    Vector2 p0, p1, p2;
} Saved_Shape;

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

static struct {
    Shapes shapes;
    Shape copied_shape;
    Shape_Type preview_shape;
    Color preview_color;
    Edit_Mode mode;
    New_Shape_Substate new_shape_substate;
    Vector2 line_start;      // initial mouse click
    Vector2 tri_second_vert; // secondary mouse click (specificallly only for triangles)
    size_t highlighted_shape_index;
    Texture translate_widget_texture;
    bool skip_preview;

    struct {
        Color color;
        int width, height;

        struct {
            Vector2 center;
            float radius;
            float value; // V in HSV
        } color_wheel;

    } panel;
} app = {0};

void configure_ui()
{
    app.panel.color      = GRAY;
    app.panel.width      = 125;
    app.panel.height     = GetScreenHeight();
    app.panel.color_wheel.value  = 1.0;
    app.panel.color_wheel.radius = 55.0f;
    app.panel.color_wheel.center = (Vector2){app.panel.width/2.0f, app.panel.width/2.0f};
    app.translate_widget_texture  = LoadTexture("assets/translate_32x32.png");
    app.preview_color = BLUE; // default preview color
}

void draw_side_panel()
{
    unsigned int tri_count = 64;
    float radius   = app.panel.color_wheel.radius;
    Vector2 center = app.panel.color_wheel.center;
    float value    = app.panel.color_wheel.value;

    // Begin rendering color wheel
    rlBegin(RL_TRIANGLES); {
        for (unsigned int i = 0; i < tri_count; i++) {
            float angle_offset = ((PI*2.0f)/(float)tri_count);
            float angle = angle_offset*(float)i;
            float angle_offset_calculated = ((float)i + 1)*angle_offset;
            Vector2 scale = (Vector2){ radius, radius };

            Vector2 offset = Vector2Multiply((Vector2){ sinf(angle), -cosf(angle) }, scale);
            Vector2 offset2 = Vector2Multiply((Vector2){ sinf(angle_offset_calculated), -cosf(angle_offset_calculated) }, scale);

            Vector2 position = Vector2Add(center, offset);
            Vector2 position2 = Vector2Add(center, offset2);

            float angle_non_radian = (angle/(2.0f*PI))*360.0f;
            float angle_non_radian_offset = (angle_offset/(2.0f*PI))*360.0f;

            Color currentColor = ColorFromHSV(angle_non_radian, 1.0f, 1.0f);
            Color offsetColor = ColorFromHSV(angle_non_radian + angle_non_radian_offset, 1.0f, 1.0f);

            rlColor4ub(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
            rlVertex2f(position.x, position.y);
            rlColor4f(value, value, value, 1.0f);
            rlVertex2f(center.x, center.y);
            rlColor4ub(offsetColor.r, offsetColor.g, offsetColor.b, offsetColor.a);
            rlVertex2f(position2.x, position2.y);
        }
    } rlEnd();

    DrawCircleLinesV(center, radius, BLACK);
}

Shape create_shape(Shape_Type type)
{
    Vector2 mouse_pos = GetMousePosition();
    float radius = Vector2Length(Vector2Subtract(mouse_pos, app.line_start));
    Shape shape = {
        .type = type,
        .color = app.preview_color,
        .rect = {
            .x = app.line_start.x,
            .y = app.line_start.y,
            .width  = mouse_pos.x - app.line_start.x,
            .height = mouse_pos.y - app.line_start.y,
        },
    };
    switch (type) {
    case SHAPE_SQUARE: {
        shape.rect.x -= radius*0.5*sqrt(2);
        shape.rect.y -= radius*0.5*sqrt(2);
        shape.rect.x += 0.5; // avoid shifting of preview shape when this reduces to an integer
        shape.rect.y += 0.5;
        shape.rect.width = radius*sqrt(2);
        shape.rect.height = radius*sqrt(2);
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            DrawRectangle(0, 0, shape.rect.width, shape.rect.height, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_RECTANGLE: {
        if (shape.rect.width < 0) {
            shape.rect.x += shape.rect.width;
            shape.rect.width *= -1.0;
        }
        if (shape.rect.height < 0) {
            shape.rect.y += shape.rect.height;
            shape.rect.height *= -1.0;
        }
        shape.rect.x += 0.5; // avoid shifting of preview shape when this reduces to an integer
        shape.rect.y += 0.5;
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            DrawRectangle(0, 0, shape.rect.width, shape.rect.height, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_CIRCLE: {
        shape.radius0 = radius;
        shape.position = app.line_start;
        shape.rect = (Rectangle) {
            .x = app.line_start.x - radius,
            .y = app.line_start.y - radius,
            .width  = 2*radius,
            .height = 2*radius,
        };
        shape.rect.x += 0.5; // avoid shifting of preview shape when this reduces to an integer
        shape.rect.y += 0.5;
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            Vector2 center = {
                .x = shape.rect.width/2,
                .y = shape.rect.height/2,
            };
            DrawCircleV(center, shape.radius0, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_ELLIPSE: {
        shape.radius0 = shape.rect.width;
        shape.radius1 = shape.rect.height;
        shape.position = app.line_start;
        shape.rect = (Rectangle) {
            .x = app.line_start.x - shape.rect.width,
            .y = app.line_start.y - shape.rect.height,
            .width  = 2*shape.rect.width,
            .height = 2*shape.rect.height,
        };
        if (shape.rect.width < 0) {
            shape.rect.x += shape.rect.width;
            shape.rect.width *= -1.0;
        }
        if (shape.rect.height < 0) {
            shape.rect.y += shape.rect.height;
            shape.rect.height *= -1.0;
        }
        shape.rect.x += 0.5; // avoid shifting of preview shape when this reduces to an integer
        shape.rect.y += 0.5;
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            Vector2 center = {
                .x = shape.rect.width/2,
                .y = shape.rect.height/2,
            };
            DrawEllipse(center.x, center.y, shape.radius0, shape.radius1, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_TRIANGLE: {
        shape.p0 = app.line_start;
        shape.p1 = app.tri_second_vert;
        shape.p2 = mouse_pos;
        float min_x = MIN(MIN(shape.p0.x, shape.p1.x), shape.p2.x);
        float min_y = MIN(MIN(shape.p0.y, shape.p1.y), shape.p2.y);
        float max_x = MAX(MAX(shape.p0.x, shape.p1.x), shape.p2.x);
        float max_y = MAX(MAX(shape.p0.y, shape.p1.y), shape.p2.y);
        shape.rect = (Rectangle){
            .x = min_x,
            .y = min_y,
            .width  = max_x - min_x + 1,
            .height = max_y - min_y + 1,
        };
        shape.rect.x += 0.5; // avoid shifting of preview shape when this reduces to an integer
        shape.rect.y += 0.5;
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            Vector2 rect_pos  = {shape.rect.x, shape.rect.y};
            Vector2 to_origin = Vector2Negate(rect_pos);
            Vector2 draw_p0 = Vector2Add(shape.p0, to_origin);
            Vector2 draw_p1 = Vector2Add(shape.p1, to_origin);
            Vector2 draw_p2 = Vector2Add(shape.p2, to_origin);
            DrawTriangle(draw_p0, draw_p1, draw_p2, shape.color);
        } EndTextureMode();
    } break;
    default: UNREACHABLE("shape unrecognized");
    }

    return shape;
}

Shape create_shape_from_saved(Saved_Shape saved_shape)
{
    Shape shape = {
        .type = saved_shape.type,
        .radius0 = saved_shape.radius0,
        .radius1 = saved_shape.radius1,
        .position = saved_shape.position,
        .rect = saved_shape.rect,
        .color = saved_shape.color,
        .p0 = saved_shape.p0,
        .p1 = saved_shape.p1,
        .p2 = saved_shape.p2,
    };
    switch (shape.type) {
    case SHAPE_SQUARE: {
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            DrawRectangle(0, 0, shape.rect.width, shape.rect.height, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_RECTANGLE: {
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            DrawRectangle(0, 0, shape.rect.width, shape.rect.height, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_CIRCLE: {
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            Vector2 center = {
                .x = shape.rect.width/2,
                .y = shape.rect.height/2,
            };
            DrawCircleV(center, shape.radius0, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_ELLIPSE: {
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            Vector2 center = {
                .x = shape.rect.width/2,
                .y = shape.rect.height/2,
            };
            DrawEllipse(center.x, center.y, shape.radius0, shape.radius1, shape.color);
        } EndTextureMode();
    } break;
    case SHAPE_TRIANGLE: {
        shape.target = LoadRenderTexture(shape.rect.width, shape.rect.height);
        BeginTextureMode(shape.target); {
            ClearBackground(BLANK);
            Vector2 rect_pos  = {shape.rect.x, shape.rect.y};
            Vector2 to_origin = Vector2Negate(rect_pos);
            Vector2 draw_p0 = Vector2Add(shape.p0, to_origin);
            Vector2 draw_p1 = Vector2Add(shape.p1, to_origin);
            Vector2 draw_p2 = Vector2Add(shape.p2, to_origin);
            DrawTriangle(draw_p0, draw_p1, draw_p2, shape.color);
        } EndTextureMode();
    } break;
    default: UNREACHABLE("shape unrecognized");
    }

    return shape;
}

void draw_shapes()
{
    for (size_t i = 0; i < app.shapes.count; i++) {
        Shape shape = app.shapes.items[i];

        DrawTexturePro(shape.target.texture,
                       (Rectangle){ 0, 0, shape.target.texture.width, -shape.target.texture.height },
                       (Rectangle){ shape.rect.x, shape.rect.y, shape.rect.width,  shape.rect.height },
                       (Vector2){0.0f, 0.0f}, 0, WHITE);
    }

    for (size_t i = 0; i < app.shapes.count; i++) {
        Shape shape = app.shapes.items[i];
        if (shape.clicked) {
            DrawRectangleLines(shape.rect.x, shape.rect.y, shape.rect.width, shape.rect.height,
                               shape.highlighted ? YELLOW : BLACK);
        }
    }
}

void draw_shape_preview()
{
    if (app.mode != EDIT_MODE_NEW_SHAPE) return;

    Color preview_color = app.preview_color;
    preview_color.a = 200; // give preview shape some transparency
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

// returns the current preview color if not hovered over color wheel
Color get_hovered_color_wheel_color()
{
    Vector2 mouse_pos = GetMousePosition();
    Color color = app.preview_color;
    float radius   = app.panel.color_wheel.radius;
    Vector2 center = app.panel.color_wheel.center;
    float value    = app.panel.color_wheel.value;

    if (CheckCollisionPointCircle(mouse_pos, center, radius)) {
        Vector2 a = Vector2Subtract(mouse_pos, center);
        float sat = Vector2Length(a)/radius;
        a         = Vector2Normalize(a);
        Vector2 b = {1.0f, 0.0f};
        float dot = Vector2DotProduct(a, b);
        float hue = 0; // will be based on the angle inside of the color wheel [0, 360]
        if (dot >= 0) {
            if (mouse_pos.y <= center.y)
                hue = dot*90;
            else
                hue = (1.0 - dot)*90 + 90;
        } else {
            dot *= -1.0;
            if (mouse_pos.y <= center.y)
                hue = 270 + 90*(1.0 - dot);
            else
                hue = 180 + 90*dot;
        }
        color = ColorFromHSV(hue, sat, value);
    }

    return color;
}

void draw_shape_preview_icon()
{
    Vector2 mouse_pos = GetMousePosition();
    Vector2 shape_icon = {mouse_pos.x + 25, mouse_pos.y - 10};
    Vector2 t0 = Vector2Add(shape_icon, (Vector2){0, 25});
    Vector2 t1 = Vector2Add(t0, (Vector2){30, 0});
    Vector2 t2 = Vector2Add(t0, (Vector2){15, -25});
    Color preview_color = get_hovered_color_wheel_color();

    if (app.mode == EDIT_MODE_NEW_SHAPE && app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
        switch (app.preview_shape) {
        case SHAPE_SQUARE:
            DrawRectangle(shape_icon.x, shape_icon.y, 25, 25, preview_color);
        break;
        case SHAPE_RECTANGLE:
            DrawRectangle(shape_icon.x, shape_icon.y, 50, 25, preview_color);
        break;
        case SHAPE_CIRCLE:
            DrawCircle(shape_icon.x + 13, shape_icon.y + 13, 13, preview_color);
        break;
        case SHAPE_ELLIPSE:
            DrawEllipse(shape_icon.x + 26, shape_icon.y + 13, 26, 13, preview_color);
        break;
        case SHAPE_TRIANGLE:
            DrawTriangle(t0, t1, t2, preview_color);
        break;
        default:
        }
    }
}

typedef struct {
    struct {
        Rectangle     top_left_bounding_box;
        Rectangle    top_right_bounding_box;
        Rectangle  bottom_left_bounding_box;
        Rectangle bottom_right_bounding_box;
    } scale;
    Rectangle translate_bounding_box;
} Edit_Widget_Info;

Edit_Widget_Info get_highlighted_shape_edit_widget_info()
{
    Edit_Widget_Info widget_info = {0};
    Shape shape = {0};
    bool found_highlighted = false;
    for (size_t i = 0; i < app.shapes.count; i++) {
        shape = app.shapes.items[i];
        if (shape.highlighted) {
            found_highlighted = true;
            break;
        }
    }

    if (!found_highlighted) {
        printf("WARNING: failed to find highlighted shape necessary for widget info\n");
        return widget_info;
    }

    // calculate bounding boxes for edit widget info
    int side = MIN_WIDTH;
    widget_info.scale.top_left_bounding_box     = (Rectangle){shape.rect.x                          , shape.rect.y, side, side};
    widget_info.scale.top_right_bounding_box    = (Rectangle){shape.rect.x - side + shape.rect.width, shape.rect.y, side, side};
    widget_info.scale.bottom_left_bounding_box  = (Rectangle){shape.rect.x                          , shape.rect.y + shape.rect.height - side, side, side};
    widget_info.scale.bottom_right_bounding_box = (Rectangle){shape.rect.x - side + shape.rect.width, shape.rect.y + shape.rect.height - side, side, side};
    widget_info.translate_bounding_box = (Rectangle){
        shape.rect.x + shape.rect.width/2  - app.translate_widget_texture.width/2,
        shape.rect.y + shape.rect.height/2 - app.translate_widget_texture.height/2,
        app.translate_widget_texture.width,
        app.translate_widget_texture.width,
    };

    return widget_info;
}

void draw_highlighted_shape_edit_widgets()
{
    Edit_Widget_Info widget_info = get_highlighted_shape_edit_widget_info();

    Rectangle t = widget_info.translate_bounding_box;
    DrawTexture(app.translate_widget_texture, t.x, t.y, WHITE);
    DrawRectangleRec(widget_info.scale.top_left_bounding_box,     BLACK);
    DrawRectangleRec(widget_info.scale.top_right_bounding_box,    BLACK);
    DrawRectangleRec(widget_info.scale.bottom_left_bounding_box,  BLACK);
    DrawRectangleRec(widget_info.scale.bottom_right_bounding_box, BLACK);

#ifdef SHOW_DEBUG_BOUNDING_BOXES
    // bounding box for scaling widgets
    DrawRectangleLinesEx(widget_info.scale.top_left_bounding_box,     1.0, RED);
    DrawRectangleLinesEx(widget_info.scale.top_right_bounding_box,    1.0, RED);
    DrawRectangleLinesEx(widget_info.scale.bottom_left_bounding_box,  1.0, RED);
    DrawRectangleLinesEx(widget_info.scale.bottom_right_bounding_box, 1.0, RED);

    // bounding box for translate widget
    DrawRectangleLinesEx(widget_info.translate_bounding_box, 1.0, RED);
#endif // SHOW_DEBUG_BOUNDING_BOXES
}

void deselect_all_edit_widgets(Shape *shape)
{
    shape->selected_translate    = false;
    shape->selected_top_left     = false;
    shape->selected_top_right    = false;
    shape->selected_bottom_left  = false;
    shape->selected_bottom_right = false;
}

int main(int argc, char **argv)
{
    const int width  = 800;
    const int height = 600;
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(width, height, "Minimal Shapes");
    configure_ui();

    String_Builder sb = {0};

    if (argc > 1) {
        const char *save_file = argv[1];
        if (!read_entire_file(save_file, &sb)) {
            printf("ERROR failed to load %s\n", save_file);
        } else {
            Saved_Shape *saved_shapes = (Saved_Shape *)sb.items;
            size_t saved_shape_count = sb.count/sizeof(Saved_Shape);
            for (size_t i = 0; i < saved_shape_count; i++) {
                da_append(&app.shapes, create_shape_from_saved(saved_shapes[i]));
            }
        }
    }

    struct {
        Shape **items;
        size_t count;
        size_t capacity;
    } clicked_shapes = {0};

    while(!WindowShouldClose()) {
        Vector2 mouse_pos = GetMousePosition();
        bool on_color_wheel = CheckCollisionPointCircle(mouse_pos, app.panel.color_wheel.center, app.panel.color_wheel.radius);
        bool on_canvas = !on_color_wheel;
        float wheel = GetMouseWheelMove();

        if (IsKeyPressed(KEY_X) && IsKeyDown(KEY_LEFT_CONTROL)) {
            // remove the highlighted shape from the list, but preserve order
            size_t highlighted_idx = 0;
            bool highlighted_found = false;
            for (size_t i = 0; i < app.shapes.count; i++) {
                if (app.shapes.items[i].highlighted) {
                    highlighted_found = true;
                    highlighted_idx = i;
                    app.copied_shape = app.shapes.items[i];
                    break;
                }
            }
            if (highlighted_found) {
                for (size_t i = highlighted_idx + 1; i < app.shapes.count; i++)
                    app.shapes.items[i-1] = app.shapes.items[i];
                app.shapes.count--;
                app.mode = EDIT_MODE_NEW_SHAPE;
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
                clicked_shapes.count = 0;
            }
        }
        if (IsKeyPressed(KEY_C) && IsKeyDown(KEY_LEFT_CONTROL)) {
            for (size_t i = 0; i < app.shapes.count; i++) {
                if (app.shapes.items[i].highlighted) {
                    app.copied_shape = app.shapes.items[i];
                    break;
                }
            }
        }
        if (IsKeyPressed(KEY_V) && IsKeyDown(KEY_LEFT_CONTROL)) {
            Shape shape = app.copied_shape;
            shape.rect.x = mouse_pos.x;
            shape.rect.y = mouse_pos.y;
            shape.clicked = false;
            shape.highlighted = false;
            da_append(&app.shapes, shape);
        }
        if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
            /* NOTE that I'm not unloading the texture, but this should happen here.
             * For now, I'm not going to worry about it */
            // remove the highlighted shape from the list, but preserve order
            size_t highlighted_idx = 0;
            bool highlighted_found = false;
            for (size_t i = 0; i < app.shapes.count; i++) {
                if (app.shapes.items[i].highlighted) {
                    highlighted_found = true;
                    highlighted_idx = i;
                    break;
                }
            }
            if (highlighted_found) {
                for (size_t i = highlighted_idx + 1; i < app.shapes.count; i++)
                    app.shapes.items[i-1] = app.shapes.items[i];
                app.shapes.count--;
                app.mode = EDIT_MODE_NEW_SHAPE;
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
                clicked_shapes.count = 0;
            }
        }
        if (IsKeyPressed(KEY_N) && IsKeyDown(KEY_LEFT_CONTROL)) {
            app.mode = EDIT_MODE_NEW_SHAPE;
            app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
            app.shapes.count = 0;
            clicked_shapes.count = 0;
        }
        if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) {
            String_Builder sb = {0};
            for (size_t i = 0; i < app.shapes.count; i++) {
                Saved_Shape saved_shape = {0};
                Shape shape = app.shapes.items[i];
                saved_shape.type     = shape.type;
                saved_shape.radius0  = shape.radius0;
                saved_shape.radius1  = shape.radius1;
                saved_shape.position = shape.position;
                saved_shape.rect     = shape.rect;
                saved_shape.color    = shape.color;
                saved_shape.p0       = shape.p0;
                saved_shape.p1       = shape.p1;
                saved_shape.p2       = shape.p2;

                sb_append_buf(&sb, &saved_shape, sizeof(saved_shape));
            }

            if (sb.count) {
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                const char *save_file = temp_sprintf("%d-%02d-%02d_%02d:%02d:%02d.bin",
                                                     tm.tm_year + 1900, tm.tm_mon + 1,
                                                     tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                if (!write_entire_file(save_file, sb.items, sb.count)) return 1;
                else printf("saved %s (%zu bytes)\n", save_file, sb.count);

                const char *png_file = temp_sprintf("%d-%02d-%02d_%02d:%02d:%02d.png",
                                                    tm.tm_year + 1900, tm.tm_mon + 1,
                                                    tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                Image image = LoadImageFromScreen();
                int force_comp = 4;
                int result = stbi_write_png(png_file, image.width, image.height, force_comp, image.data, 0);
                if (!result) {
                    printf("ERROR: failed to save %s\n", png_file);
                } else {
                    printf("saved image %s\n", png_file);
                }

                // target = LoadRenderTexture(800, 600);
                // BeginTextureMode(shape.target); {
                //     ClearBackground(WHITE);
                //     draw_shapes();
                // } EndTextureMode();
                // SaveTextureAsImage

                sb_free(sb);
            }
        }

        if (wheel != 0.0  && app.mode == EDIT_MODE_NEW_SHAPE &&
            app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {

            // if on canvs change preview shape, otherwise change the V in HSV
            if (on_canvas) {
                if (wheel < 0) app.preview_shape = (app.preview_shape + SHAPE_COUNT - 1)%SHAPE_COUNT;
                else           app.preview_shape = (app.preview_shape + 1)%SHAPE_COUNT;
            } else {
                if (wheel < 0) {
                    float nv = app.panel.color_wheel.value - 0.1;
                    app.panel.color_wheel.value = (nv < 0.0f) ? 0.0f : nv;
                } else {
                    float nv = app.panel.color_wheel.value + 0.1;
                    app.panel.color_wheel.value = (nv > 1.0f) ? 1.0f : nv;
                }
            }
        }

        /* handle color select */
        if (!on_canvas && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app.mode = EDIT_MODE_NEW_SHAPE;
            app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
            app.preview_color = get_hovered_color_wheel_color();
            for (size_t i = 0; i < clicked_shapes.count; i++) {
                clicked_shapes.items[i]->highlighted = false;
                clicked_shapes.items[i]->clicked = false;
            }
            clicked_shapes.count = 0;
        }

        /* handle select mode */
        if (on_canvas && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
            clicked_shapes.count = 0;
            for (size_t i = app.shapes.count; i > 0; i--) {
                Shape *shape = &app.shapes.items[i-1];
                if ((shape->clicked = CheckCollisionPointRec(mouse_pos, shape->rect))) {
                    da_append(&clicked_shapes, shape);
                } else {
                    shape->highlighted = false;
                }
            }
            Edit_Mode prev_mode = app.mode;
            app.mode = (clicked_shapes.count) ? EDIT_MODE_SELECT : EDIT_MODE_NEW_SHAPE;

            /* if we just changed to selcted, then skip drawing the preview shape this frame */
            if (app.mode == EDIT_MODE_NEW_SHAPE && prev_mode == EDIT_MODE_SELECT) {
                app.skip_preview = true;
            }
        }

        // highlight at least one of the clicked shapes, if any
        bool any_shapes_highlighted = false;
        for (size_t i = 0; i < clicked_shapes.count; i++) {
            if (clicked_shapes.items[i]->highlighted) {
                any_shapes_highlighted = true;
                app.highlighted_shape_index = i;
            }
        }
        if (!any_shapes_highlighted && clicked_shapes.count) {
            clicked_shapes.items[0]->highlighted = true;
            app.highlighted_shape_index = 0;
        }

        // if there are multiple clicked shapes use scroll wheel to switch movable shape (i.e. highlighted)
        if (wheel != 0.0  && app.mode == EDIT_MODE_SELECT) {
            if (clicked_shapes.count) {
                size_t count = clicked_shapes.count;
                if (wheel < 0) app.highlighted_shape_index = (app.highlighted_shape_index + count - 1)%count;
                else           app.highlighted_shape_index = (app.highlighted_shape_index + 1)%count;
                for (size_t i = 0; i < clicked_shapes.count; i++) {
                    Shape *shape = clicked_shapes.items[i];
                    shape->highlighted = app.highlighted_shape_index == i;
                }
            }
        }

        /* handle resizing and translation */
        if (app.mode == EDIT_MODE_SELECT && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && on_canvas) {
            for (size_t i = 0; i < app.shapes.count; i++) {
                Shape *shape = &app.shapes.items[i];
                if (shape->highlighted) {
                    Edit_Widget_Info widget_info = get_highlighted_shape_edit_widget_info();

                    // first check to see if we are already selecting something
                    if (shape->selected_translate) {
                        shape->rect.x = mouse_pos.x - shape->rect.width/2;
                        shape->rect.y = mouse_pos.y - shape->rect.height/2;
                        continue;
                    } else if (shape->selected_top_left) {
                        float nx = mouse_pos.x - widget_info.scale.top_left_bounding_box.width/2;
                        float ny = mouse_pos.y - widget_info.scale.top_left_bounding_box.height/2;
                        float dx = shape->rect.x - nx;
                        float dy = shape->rect.y - ny;
                        float nw = shape->rect.width  + dx;
                        float nh = shape->rect.height + dy;
                        if (nw <= MIN_WIDTH || nh <= MIN_HEIGHT) continue;

                        shape->rect.x      = nx;
                        shape->rect.y      = ny;
                        shape->rect.width  = nw;
                        shape->rect.height = nh;
                        continue;
                    } else if (shape->selected_top_right) {
                        float nx = mouse_pos.x + widget_info.scale.top_right_bounding_box.width/2 - shape->rect.width;
                        float ny = mouse_pos.y - widget_info.scale.top_right_bounding_box.height/2;
                        float dx = nx - shape->rect.x;
                        float dy = shape->rect.y - ny;
                        float nw = shape->rect.width  + dx;
                        float nh = shape->rect.height + dy;
                        if (nw <= MIN_WIDTH || nh <= MIN_HEIGHT) continue;

                        shape->rect.y      = ny;
                        shape->rect.width  = nw;
                        shape->rect.height = nh;
                        continue;
                    } else if (shape->selected_bottom_left) {
                        float nx = mouse_pos.x - widget_info.scale.top_left_bounding_box.width/2;
                        float ny = mouse_pos.y + widget_info.scale.top_left_bounding_box.height/2 - shape->rect.height;
                        float dx = shape->rect.x - nx;
                        float dy = ny - shape->rect.y; // TODO: weird???
                        float nw = shape->rect.width  + dx;
                        float nh = shape->rect.height + dy;

                        if (nw <= MIN_WIDTH || nh <= MIN_HEIGHT) continue;
                        shape->rect.x      = nx;
                        shape->rect.width  = nw;
                        shape->rect.height = nh;
                        continue;
                    } else if (shape->selected_bottom_right) {
                        float nw = mouse_pos.x - shape->rect.x + widget_info.scale.bottom_right_bounding_box.width/2;
                        float nh = mouse_pos.y - shape->rect.y + widget_info.scale.bottom_right_bounding_box.height/2;
                        if (nw <= MIN_WIDTH || nh <= MIN_HEIGHT) continue;
                        shape->rect.width  = nw;
                        shape->rect.height = nh;
                        continue;
                    }

                    // if nothing was selected, then check for collision on one of the widgets
                    if (CheckCollisionPointRec(mouse_pos, widget_info.translate_bounding_box)) {
                        deselect_all_edit_widgets(shape);
                        shape->selected_translate = true;
                    } else if (CheckCollisionPointRec(mouse_pos, widget_info.scale.top_left_bounding_box)) {
                        deselect_all_edit_widgets(shape);
                        shape->selected_top_left = true;
                    } else if (CheckCollisionPointRec(mouse_pos, widget_info.scale.bottom_left_bounding_box)) {
                        deselect_all_edit_widgets(shape);
                        shape->selected_bottom_left = true;
                    } else if (CheckCollisionPointRec(mouse_pos, widget_info.scale.bottom_right_bounding_box)) {
                        deselect_all_edit_widgets(shape);
                        shape->selected_bottom_right = true;
                    } else if (CheckCollisionPointRec(mouse_pos, widget_info.scale.top_right_bounding_box)) {
                        deselect_all_edit_widgets(shape);
                        shape->selected_top_right = true;
                    }
                }
            }
        } else if (app.mode == EDIT_MODE_SELECT && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < app.shapes.count; i++)
                deselect_all_edit_widgets(&app.shapes.items[i]);
        }

        // reset or cancel current shape draw
        if (app.mode == EDIT_MODE_NEW_SHAPE && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
        }

        if (on_canvas && app.mode == EDIT_MODE_NEW_SHAPE && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !app.skip_preview) {
            if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_PREVIEW) {
                app.line_start = mouse_pos;
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_LINE_1;
            } else if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_1) {
                if (app.preview_shape == SHAPE_TRIANGLE) {
                    app.new_shape_substate = NEW_SHAPE_SUBSTATE_LINE_2;
                } else {
                    app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
                    da_append(&app.shapes, create_shape(app.preview_shape));
                }
            } else if (app.new_shape_substate == NEW_SHAPE_SUBSTATE_LINE_2) {
                app.new_shape_substate = NEW_SHAPE_SUBSTATE_PREVIEW;
                da_append(&app.shapes, create_shape(app.preview_shape));
            }
        }

        BeginDrawing(); {
            ClearBackground(WHITE);
            rlDisableBackfaceCulling();

            draw_shapes();

            if (app.mode == EDIT_MODE_SELECT)
                draw_highlighted_shape_edit_widgets();

            draw_side_panel();

            if (app.mode == EDIT_MODE_NEW_SHAPE && !app.skip_preview) {
                draw_shape_preview();
                draw_shape_preview_icon();
            }

        } EndDrawing();

        app.skip_preview = false;
    }
    CloseWindow();
    return 0;
}
