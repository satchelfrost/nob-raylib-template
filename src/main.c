#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define NOB_IMPLEMNTATION
#include "../nob.h"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

#define RECT_LEN 20.0f
#define PADDING 5.0f
#define LR_PADDING 50.0f

typedef struct {
    Vector2 pos;
    Vector2 size;
    Color color;
} Square;

typedef struct {
    Square *items;
    size_t count;
    size_t capacity;
} Squares;

void create_squares(Squares *squares, size_t count)
{
    float cell_len = (WINDOW_WIDTH - 2.0f * LR_PADDING) / count;
    float squ_len  = cell_len - PADDING;

    for (size_t i = 0; i < count; i++) {
        Square squ = {
            .pos = {
                .x = LR_PADDING + 0.5f * PADDING + i * cell_len,
                .y = WINDOW_HEIGHT / 2.0f - squ_len / 2.0f,
            },
            .size = {
                .x = squ_len,
                .y = squ_len,
            },
            .color = BLACK,
        };
        nob_da_append(squares, squ);
    }
}

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hello from raylib");

    Squares squares = {0};
    create_squares(&squares, 10);

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
            for (size_t i = 0; i < squares.count; i++) {
                Square squ = squares.items[i];
                DrawRectangleV(squ.pos, squ.size, squ.color);
            }

            float debug_len = LR_PADDING + 0.5f * PADDING;
            Vector2 debug_pos = {
                .x = 0,
                .y = WINDOW_WIDTH / 2.0f - (LR_PADDING + PADDING) * 0.5f,
            };
            DrawRectangleV(debug_pos, (Vector2){.x = debug_len, .y = debug_len}, RED);
            debug_pos.x = WINDOW_WIDTH - debug_len;
            DrawRectangleV(debug_pos, (Vector2){.x = debug_len, .y = debug_len}, RED);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
