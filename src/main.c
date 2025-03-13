#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define NOB_IMPLEMNTATION
#include "../nob.h"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

#define BOX_PADDING 5.0f
#define BORDER_PADDING 50.0f
#define INSTR_PADDING 15.0f
#define BEAVER_HEIGHT 10.0f
#define FONT_SIZE 50

typedef struct {
    Vector2 pos;
    Vector2 size;
    Color color;
    int value;
} Square;

typedef struct {
    Square *items;
    size_t count;
    size_t capacity;
} Squares;

typedef enum {
    BB_STATE_A,
    BB_STATE_B,
    BB_STATE_C,
    BB_STATE_D,
    BB_STATE_HALT,
} BB_State;

typedef struct {
    size_t write;
    int left_right;
    BB_State next_state;
} Instruction;

typedef struct {
    Instruction instrs[2];
} Bi_Instruction;

Bi_Instruction bb_1_instructions[] = {
    [BB_STATE_A] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_HALT,
        },
        (Instruction) { // one instr
            .write = 0,
            .left_right = 0,
            .next_state = BB_STATE_HALT,
        },
    },
};

Bi_Instruction bb_2_instructions[] = {
    [BB_STATE_A] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_B,
        },
        (Instruction) { // one instr
            .write = 1,
            .left_right = -1,
            .next_state = BB_STATE_B,
        },
    },
    [BB_STATE_B] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = -1,
            .next_state = BB_STATE_A,
        },
        (Instruction) { // one instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_HALT,
        },
    },
};

Bi_Instruction bb_3_instructions[] = {
    [BB_STATE_A] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_B,
        },
        (Instruction) { // one instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_HALT,
        },
    },
    [BB_STATE_B] = {
        (Instruction) { // zero instr
            .write = 0,
            .left_right = 1,
            .next_state = BB_STATE_C,
        },
        (Instruction) { // one instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_B,
        },
    },
    [BB_STATE_C] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = -1,
            .next_state = BB_STATE_C,
        },
        (Instruction) { // one instr
            .write = 1,
            .left_right = -1,
            .next_state = BB_STATE_A,
        },
    },
};

Bi_Instruction bb_4_instructions[] = {
    [BB_STATE_A] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_B,
        },
        (Instruction) { // one instr
            .write = 1,
            .left_right = -1,
            .next_state = BB_STATE_B,
        },
    },
    [BB_STATE_B] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = -1,
            .next_state = BB_STATE_A,
        },
        (Instruction) { // one instr
            .write = 0,
            .left_right = -1,
            .next_state = BB_STATE_C,
        },
    },
    [BB_STATE_C] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_HALT,
        },
        (Instruction) { // one instr
            .write = 1,
            .left_right = -1,
            .next_state = BB_STATE_D,
        },
    },
    [BB_STATE_D] = {
        (Instruction) { // zero instr
            .write = 1,
            .left_right = 1,
            .next_state = BB_STATE_D,
        },
        (Instruction) { // one instr
            .write = 0,
            .left_right = 1,
            .next_state = BB_STATE_A,
        },
    },
};

void create_squares(Squares *squares, size_t count)
{
    float cell_len = (WINDOW_WIDTH - 2.0f * BORDER_PADDING) / count;
    float squ_len  = cell_len - BOX_PADDING;

    for (size_t i = 0; i < count; i++) {
        Square squ = {
            .pos = {
                .x = BORDER_PADDING + 0.5f * BOX_PADDING + i * cell_len,
                .y = WINDOW_HEIGHT / 2.6,
            },
            .size = {
                .x = squ_len,
                .y = squ_len,
            },
            .value = 0,
        };
        nob_da_append(squares, squ);
    }
}

const char *state_to_str(BB_State state, bool as_char)
{
    switch (state) {
    case BB_STATE_A:    return "A";
    case BB_STATE_B:    return "B";
    case BB_STATE_C:    return "C";
    case BB_STATE_HALT: return (as_char) ? "H" : "Halt";
    case BB_STATE_D:    return "D";
    default:            return "Unreachable";
    }
}

const char *lr_as_str(int lr)
{
    return (lr == -1) ? "L" : "R";
}

typedef struct {
    size_t box_count;
    size_t idx;
    const char *file;
    Bi_Instruction *bi_instructions;
} BB;

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hello from raylib");

    /* Different halting busy beavers */
    BB bbs[4] = {
        {
            .box_count = 5,
            .idx = 2,
            .file = "bb1-card.png",
            .bi_instructions = bb_1_instructions
        },
        {
            .box_count = 8,
            .idx = 4,
            .file = "bb2-card.png",
            .bi_instructions = bb_2_instructions
        },
        {
            .box_count = 10,
            .idx = 3,
            .file = "bb3-card.png",
            .bi_instructions = bb_3_instructions
        },
        {
            .box_count = 18,
            .idx = 12,
            .file = "bb4-card.png",
            .bi_instructions = bb_4_instructions
        },
    };

    Squares squares = {0};
    size_t busy_beaver_mach_idx = 0; // current busy beaver machine
    Bi_Instruction *bi_instructions = bbs[busy_beaver_mach_idx].bi_instructions;
    create_squares(&squares, bbs[busy_beaver_mach_idx].box_count);
    size_t beaver_idx = bbs[busy_beaver_mach_idx].idx;
    BB_State state = BB_STATE_A; // All bbs begin in state A
    Square beaver = {.color = GRAY, .size = {.y = BEAVER_HEIGHT}};
    Texture2D bb_card_texture = LoadTexture(bbs[busy_beaver_mach_idx].file);
    size_t count = 0;

    while(!WindowShouldClose()) {
        /* update the busy beaver */
        if (IsKeyPressed(KEY_SPACE) && state != BB_STATE_HALT) {
            int tape_value = squares.items[beaver_idx].value;
            Instruction instr = bi_instructions[state].instrs[tape_value];
            squares.items[beaver_idx].value = instr.write;
            beaver_idx += instr.left_right;
            state = instr.next_state;
            printf("step: %zu\n", ++count);
        }

        /* reset the state */
        if (IsKeyPressed(KEY_R)) {
            count = 0;
            state = BB_STATE_A;
            beaver_idx = bbs[busy_beaver_mach_idx].idx;
            beaver = (Square) {
                .color = GRAY,
                .size = {.y = BEAVER_HEIGHT},
            };
            squares.count = 0;
            create_squares(&squares, bbs[busy_beaver_mach_idx].box_count);
        }

        /* switch busy beaver machine */
        if (IsKeyPressed(KEY_S)) {
            busy_beaver_mach_idx = (busy_beaver_mach_idx + 1) % NOB_ARRAY_LEN(bbs);
            UnloadTexture(bb_card_texture);
            bb_card_texture = LoadTexture(bbs[busy_beaver_mach_idx].file);
            bi_instructions = bbs[busy_beaver_mach_idx].bi_instructions;

            /* reset the state just in case */
            count = 0;
            state = BB_STATE_A;
            beaver_idx = bbs[busy_beaver_mach_idx].idx;
            beaver = (Square) {
                .color = GRAY,
                .size = {.y = BEAVER_HEIGHT},
            };
            squares.count = 0;
            create_squares(&squares, bbs[busy_beaver_mach_idx].box_count);
        }

        /* draw */
        BeginDrawing();
            ClearBackground(WHITE);

            /* draw state label */
            const char *text = TextFormat("State: %s", state_to_str(state, false));
            float state_width = MeasureText(text, FONT_SIZE);
            DrawText(text, WINDOW_WIDTH / 2.0f - state_width / 2.0f, BORDER_PADDING, FONT_SIZE, BLACK);

            /* draw instructions */
            Vector2 texture_pos = {
                .x = WINDOW_WIDTH / 2.0f - bb_card_texture.width / 2.0f,
                .y = WINDOW_HEIGHT - bb_card_texture.height - BORDER_PADDING
            };
            DrawTexture(bb_card_texture, texture_pos.x, texture_pos.y, WHITE);

            /* draw tape with values */
            for (size_t i = 0; i < squares.count; i++) {
                Square squ = squares.items[i];
                DrawRectangleV(squ.pos, squ.size, BLACK);
                const char *txt = TextFormat("%d", squ.value);
                float width = MeasureText(txt, squ.size.y);
                Vector2 text_pos = {
                    .x = squ.pos.x + 0.5f * squ.size.x - 0.5f * width,
                    .y = squ.pos.y + squ.size.y * 0.05f, 
                };
                DrawText(txt, text_pos.x, text_pos.y, squ.size.y, GREEN);
            }

            /* draw beaver */
            beaver.pos.x = squares.items[beaver_idx].pos.x;
            beaver.pos.y = squares.items[beaver_idx].pos.y + squares.items[beaver_idx].size.y + BOX_PADDING;
            beaver.size.x = squares.items[beaver_idx].size.x;
            DrawRectangleV(beaver.pos, beaver.size, beaver.color);

        EndDrawing();
    }

    UnloadTexture(bb_card_texture);
    CloseWindow();
    return 0;
}
