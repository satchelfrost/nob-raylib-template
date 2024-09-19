#include "raylib.h"
#include "rcamera.h"
#include "rlgl.h"
#include "raymath.h"
#include <assert.h>

#define SCREEN_FACTOR 80
#define SCREEN_WIDTH  (16 * SCREEN_FACTOR)
#define SCREEN_HEIGHT ( 9 * SCREEN_FACTOR)
#define FONT_SIZE 22

#define KEY_SPACING 10 
#define KEY_WIDTH 100

#define MOUSE_MOVE_SENSITIVITY 0.005f

#define GAMEPAD_ROT_SENSITIVITY 1.0f
#define GAMEPAD_ROTATION_SPEED 5.0f
#define GAMEPAD_CUBE_SPEED 10.0f
#define DEAD_ZONE 0.25f

#define CUBE_SPEED 3.0f
#define NUM_TARGET_CUBES 5
#define SCALE_SPEED 0.5f
#define MIN_CUBE_SCALE 0.5f
#define MAX_CUBE_SCALE 2.0f
#define BOUNDS 10

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))

typedef struct {
    Vector3 pos;
    Vector3 forward;
    Vector3 up;
    Vector3 size;
    Color color;
    Matrix m;
} Cube;

typedef enum {
    STATE_INTRO,
    STATE_CALIBRATE_KEY_MOUSE,
    STATE_CALIBRATE_GAMEPAD,
    STATE_MOVEMENT_KEY_MOUSE,
    STATE_MOVEMENT_GAMEPAD,
    STATE_END,
    STATE_COUNT,
} State;

void update_cube_gamepad(Cube *cube);
void update_cube_key_mouse(Cube *cube);
float render_text_centered(const char *msg, int y, Color color);

State handle_state_change(State state)
{
    bool key_mouse_calibrated = false;
    bool gamepad_calibrated = true;
    bool key_mouse_state_done = false;
    bool gamepad_state_done = false;

    switch (state) {
    case STATE_INTRO:
        if (IsKeyPressed(KEY_SPACE))
            return (state + 1) % STATE_COUNT;
        break;
    case STATE_CALIBRATE_KEY_MOUSE:
        if (key_mouse_calibrated)
            return (state + 1) % STATE_COUNT;
        break;
    case STATE_CALIBRATE_GAMEPAD:
        if (gamepad_calibrated)
            return (state + 1) % STATE_COUNT;
        break;
    case STATE_MOVEMENT_GAMEPAD:
        if (gamepad_state_done)
            return (state + 1) % STATE_COUNT;
        break;
    case STATE_MOVEMENT_KEY_MOUSE:
        if (key_mouse_state_done)
            return (state + 1) % STATE_COUNT;
        break;
    case STATE_END:
    default: return state;
    }

    return state;
}

void render_key_mouse_calibrate()
{
    static bool w = false;
    static bool a = false;
    static bool s = false;
    static bool d = false;
    static bool up = false;
    static bool down = false;
    static bool mouse_right = false;

    if (!w) w = IsKeyPressed(KEY_W);
    if (!a) a = IsKeyPressed(KEY_A);
    if (!s) s = IsKeyPressed(KEY_S);
    if (!d) d = IsKeyPressed(KEY_D);
    if (!up) up = IsKeyPressed(KEY_UP);
    if (!down) down = IsKeyPressed(KEY_DOWN);
    if (!mouse_right) mouse_right = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);


    int x_offset = SCREEN_WIDTH / 4;
    const char *wasd = "W, A, S, D, for translation";
    int txt_width = MeasureText(wasd, FONT_SIZE);
    DrawText(wasd,
             x_offset - txt_width / 2,
             SCREEN_HEIGHT / 2 - FONT_SIZE - KEY_WIDTH / 2 - KEY_SPACING,
             FONT_SIZE, BLACK);

    /* W */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2  - KEY_WIDTH / 2,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2,
                  KEY_WIDTH, KEY_WIDTH, w ? GREEN: RED);
    /* A */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2  - KEY_WIDTH / 2 - KEY_WIDTH - KEY_SPACING,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  KEY_WIDTH, KEY_WIDTH, a ? GREEN : RED);
    /* S */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2  - KEY_WIDTH / 2,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  KEY_WIDTH, KEY_WIDTH, s ? GREEN : RED);
    /* D */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2  - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  KEY_WIDTH, KEY_WIDTH, d ? GREEN : RED);

    /* up/down arrow gui*/
    // const char *up_down_txt = "up/down arrows scales cube";
    // txt_width = MeasureText(up_down_txt, FONT_SIZE);
    // DrawText(up_down_txt,
    //          txt_width / 2,
    //          SCREEN_HEIGHT / 2 - FONT_SIZE - KEY_WIDTH / 2 - KEY_SPACING,
    //          FONT_SIZE, BLACK);
    // DrawRectangle(KEY_WIDTH / 2,
    //               SCREEN_HEIGHT / 2 - KEY_WIDTH / 2,
    //               KEY_WIDTH, KEY_WIDTH, up ? GREEN : RED);


    /* mouse right click gui */
    x_offset += SCREEN_WIDTH / 2;
    const char *mouse_txt = "Right click does rotation";
    txt_width = MeasureText(mouse_txt, FONT_SIZE);
    DrawText(mouse_txt,
             x_offset - txt_width / 2,
             SCREEN_HEIGHT / 2 - FONT_SIZE - KEY_WIDTH / 2 - KEY_SPACING,
             FONT_SIZE, BLACK);
    DrawRectangle(x_offset - KEY_WIDTH / 2,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2,
                  KEY_WIDTH, KEY_WIDTH, mouse_right ? GREEN : RED);
}

float render_text_centered(const char *msg, int y, Color color)
{
    int txt_width = MeasureText(msg, FONT_SIZE);
    float x = SCREEN_WIDTH / 2.0f - txt_width / 2.0f;
    DrawText(msg, x, y, FONT_SIZE, color);
    return x;
}

typedef struct {
    const char *txt;
    Color color;
    bool center;
} Text;

void render_intro()
{
    const char *intro = "Please do not press any keys until reading all instructions.";
    const char *warning_label =  "*WARNING*";
    const char *warning_1_1 = "you must have a gamepad controller that can conntect to your computer via bluetooth.";
    const char *warning_1_2 = "XBox, and Wii Pro controllers have been tested, but others may work as well. You will know if";
    const char *warning_1_3 = "it doesn't work if you are unable to make it past the gamepad calibration screen";
    const char *section_1 = "This test consists of Five parts in the following order:";
    const char *bullet_1  = "1) Keyboard + Mouse calibration / controls explanation";
    const char *bullet_2  = "2) Gamepad calibration / controls explanation";
    const char *bullet_3  = "3) Keyboard + Mouse movement of cube (x5 for each cube)";
    const char *bullet_4  = "4) Gamepad movement of cube (x5 for each cube)";
    const char *bullet_5  = "5) Results screen";
    const char *section_2_1 = "the results are not stored on this website, therefore you must screenshot";
    const char *section_2_2 = "your results, and send it to reese.gallagher@ucf.edu or post them to the discussion board";
    const char *section_2_3 = "where this link was posted";
    const char *end_msg = "When you are ready to begin press the spacebar";

    const Text msgs[] = {
        {.txt = intro, .color = BLACK, .center = true},
        {.txt = ""},
        {.txt = warning_label, .color = RED, .center = true},
        {.txt = warning_1_1, .color = BLACK, .center = true},
        {.txt = warning_1_2, .color = BLACK, .center = true},
        {.txt = warning_1_3, .color = BLACK, .center = true},
        {.txt = ""},
        {.txt = section_1, .color = BLACK, .center = true},
        {.txt = bullet_1, .color = BLACK},
        {.txt = bullet_2, .color = BLACK},
        {.txt = bullet_3, .color = BLACK},
        {.txt = bullet_4, .color = BLACK},
        {.txt = bullet_5, .color = BLACK},
        {.txt = ""},
        {.txt = warning_label, .color = RED, .center = true},
        {.txt = section_2_1, .color = BLACK, .center = true},
        {.txt = section_2_2, .color = BLACK, .center = true},
        {.txt = section_2_3, .color = BLACK, .center = true},
        {.txt = ""},
        {.txt = end_msg, .color = BLACK, .center = true},
    };

    int y = 50;
    float x = 0;
    for (int i = 0; i < (int)ARRAY_LEN(msgs); i++) {
        if (msgs[i].center)
            x = render_text_centered(msgs[i].txt, y, msgs[i].color);
        else
            DrawText(msgs[i].txt, x, y, FONT_SIZE, msgs[i].color);
        y += FONT_SIZE * 1.2f;
    }
}

void movement_state_update(State state, Cube *cube)
{
    if (state == STATE_MOVEMENT_KEY_MOUSE)    update_cube_key_mouse(cube);
    else if (state == STATE_MOVEMENT_GAMEPAD) update_cube_gamepad(cube);
    else return;

    /* bounds check */
    if (cube->pos.x < -BOUNDS || cube->pos.x > BOUNDS ||
        cube->pos.y < -BOUNDS || cube->pos.y > BOUNDS ||
        cube->pos.z < -BOUNDS || cube->pos.z > BOUNDS)
        cube->pos = Vector3Zero();

    /* update  cube matrix */
    Matrix r = MatrixInvert(MatrixLookAt((Vector3){0.0f, 0.0f, 0.0f}, cube->forward, cube->up));
    Matrix t = MatrixTranslate(cube->pos.x, cube->pos.y, cube->pos.z);
    Matrix s = MatrixScale(cube->size.x, cube->size.y, cube->size.z);
    cube->m = MatrixMultiply(s, MatrixMultiply(r, t));
}

void movement_state_render(Cube cube, Matrix *target_cubes)
{
    DrawGrid(20, 1);

    /* target cubes */
    for (int i = 0; i < NUM_TARGET_CUBES; i++) {
        rlPushMatrix();
            rlMultMatrixf(MatrixToFloatV(target_cubes[i]).v);
            DrawCubeWiresV((Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f}, BLACK);
            rlPushMatrix();
                DrawLine3D((Vector3){0.0f, 0.0f, -0.5f}, (Vector3){0.0f, 0.0f, -2.0f}, BLACK);
                rlTranslatef(0.0f, 0.0f, -2.0f);
                DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, 0.1f, BLUE);
            rlPopMatrix();
            rlPushMatrix();
                DrawLine3D((Vector3){0.0f, 0.5f, 0.0f}, (Vector3){0.0f, 2.0f, 0.0f}, BLACK);
                rlTranslatef(0.0f, 2.0f, 0.0f);
                DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, 0.1f, GREEN);
            rlPopMatrix();
            rlPushMatrix();
                DrawLine3D((Vector3){0.5f, 0.0f, 0.0f}, (Vector3){2.0f, 0.0f, 0.0f}, BLACK);
                rlTranslatef(2.0f, 0.0f, 0.0f);
                DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, 0.1f, MAGENTA);
            rlPopMatrix();
        rlPopMatrix();
    }

    /* user controlled cube */
    rlPushMatrix();
        rlMultMatrixf(MatrixToFloatV(cube.m).v);
        DrawCubeV((Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f}, cube.color);
        rlPushMatrix();
            DrawLine3D((Vector3){0.0f, 0.0f, -0.5f}, (Vector3){0.0f, 0.0f, -2.0f}, BLACK);
            rlTranslatef(0.0f, 0.0f, -2.0f);
            DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, 0.1f, BLUE);
        rlPopMatrix();
        rlPushMatrix();
            DrawLine3D((Vector3){0.0f, 0.5f, 0.0f}, (Vector3){0.0f, 2.0f, 0.0f}, BLACK);
            rlTranslatef(0.0f, 2.0f, 0.0f);
            DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, 0.1f, GREEN);
        rlPopMatrix();
        rlPushMatrix();
            DrawLine3D((Vector3){0.5f, 0.0f, 0.0f}, (Vector3){2.0f, 0.0f, 0.0f}, BLACK);
            rlTranslatef(2.0f, 0.0f, 0.0f);
            DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, 0.1f, MAGENTA);
        rlPopMatrix();
    rlPopMatrix();
}

void update_cube_gamepad(Cube *cube)
{
    float dt = GetFrameTime();
    Vector3 forward = Vector3Normalize(cube->forward);
    Vector3 up = Vector3Normalize(cube->up);
    Vector3 right = Vector3CrossProduct(forward, up);

    /* rotation */
    float joy_x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X); 
    float joy_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
    Vector3 rot_y = Vector3RotateByAxisAngle(forward, up,    -joy_x * dt * GAMEPAD_ROTATION_SPEED);
    Vector3 rot_x = Vector3RotateByAxisAngle(forward, right, -joy_y * dt * GAMEPAD_ROTATION_SPEED);
    cube->forward = Vector3Normalize(Vector3Add(rot_y, rot_x));

    /* translation */
    float speed = dt * GAMEPAD_CUBE_SPEED;
    float fb = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
    float lr = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
    float fb_norm = (fabs(fb) - DEAD_ZONE) / (1.0f - DEAD_ZONE);
    float lr_norm = (fabs(lr) - DEAD_ZONE) / (1.0f - DEAD_ZONE);
    if (fb <= -DEAD_ZONE) cube->pos = Vector3Add(cube->pos, Vector3Scale(forward,  speed * fb_norm));
    if (lr <= -DEAD_ZONE) cube->pos = Vector3Add(cube->pos, Vector3Scale(right,   -speed * lr_norm));
    if (fb >=  DEAD_ZONE) cube->pos = Vector3Add(cube->pos, Vector3Scale(forward, -speed * fb_norm));
    if (lr >=  DEAD_ZONE) cube->pos = Vector3Add(cube->pos, Vector3Scale(right,    speed * lr_norm));

    /* scaling */
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) ||
        IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)) {
        float scale = (cube->size.x + SCALE_SPEED * dt < MAX_CUBE_SCALE) ? SCALE_SPEED * dt : 0.0f;
        cube->size.x += scale;
        cube->size.y += scale;
        cube->size.z += scale;
    }
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
        IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2)) {
        float scale = (cube->size.x + SCALE_SPEED * dt > MIN_CUBE_SCALE) ? SCALE_SPEED * dt : 0.0f;
        cube->size.x -= scale;
        cube->size.y -= scale;
        cube->size.z -= scale;
    }
}

void update_cube_key_mouse(Cube *cube)
{
    float dt = GetFrameTime();
    Vector3 forward = Vector3Normalize(cube->forward);
    Vector3 up = Vector3Normalize(cube->up);
    Vector3 right = Vector3CrossProduct(forward, up);

    /* rotation */
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        Vector3 rot_y = Vector3RotateByAxisAngle(forward, up,    -delta.x * MOUSE_MOVE_SENSITIVITY);
        Vector3 rot_x = Vector3RotateByAxisAngle(forward, right, -delta.y * MOUSE_MOVE_SENSITIVITY);
        cube->forward = Vector3Normalize(Vector3Add(rot_y, rot_x));
    }

    /* translation */
    float speed = (IsKeyDown(KEY_LEFT_SHIFT)) ? dt * CUBE_SPEED * 0.25f : dt * CUBE_SPEED;
    if (IsKeyDown(KEY_W)) cube->pos = Vector3Add(cube->pos, Vector3Scale(forward,  speed));
    if (IsKeyDown(KEY_S)) cube->pos = Vector3Add(cube->pos, Vector3Scale(forward, -speed));
    if (IsKeyDown(KEY_D)) cube->pos = Vector3Add(cube->pos, Vector3Scale(right,    speed));
    if (IsKeyDown(KEY_A)) cube->pos = Vector3Add(cube->pos, Vector3Scale(right,   -speed));

    /* scaling */
    if (IsKeyDown(KEY_UP))  {
        float scale = (cube->size.x + SCALE_SPEED * dt < MAX_CUBE_SCALE) ? SCALE_SPEED * dt : 0.0f;
        cube->size.x += scale;
        cube->size.y += scale;
        cube->size.z += scale;
    }
    if (IsKeyDown(KEY_DOWN)) {
        float scale = (cube->size.x + SCALE_SPEED * dt > MIN_CUBE_SCALE) ? SCALE_SPEED * dt : 0.0f;
        cube->size.x -= scale;
        cube->size.y -= scale;
        cube->size.z -= scale;
    }
}

static Matrix default_cubes[NUM_TARGET_CUBES] = {
    {
        0.74, -0.26, -0.62, 1.55,
        0.00, 0.92, -0.38, 0.96,
        0.67, 0.28, 0.69, 5.33,
        0.00, 0.00, 0.00, 1.00,
    },
    {
        0.43, 0.32, 0.85, -0.62,
        0.00, 0.94, -0.35, 1.07,
        -0.90, 0.15, 0.40, 9.24,
        0.00, 0.00, 0.00, 1.00,
    },
    {
        1.98, -0.02, -0.28, -3.77,
        0.00, 2.00, -0.13, 0.21,
        0.28, 0.13, 1.98, -3.94,
        0.00, 0.00, 0.00, 1.00,
    },
    {
        0.38, -0.23, -1.13, 7.47,
        0.00, 1.19, -0.24, 0.14,
        1.15, 0.07, 0.37, -6.20,
        0.00, 0.00, 0.00, 1.00,
    },
    {
        0.98, 0.28, 0.43, 7.20,
        0.00, 0.93, -0.60, -2.99,
        -0.51, 0.53, 0.83, 0.36,
        0.00, 0.00, 0.00, 1.00,
    },
};

int main()
{
    Camera camera = {
        .position   = {0.0f, 3.0f, 15.0f},
        .target     = {0.0f, 0.0f, 0.0f},
        .up         = {0.0f, 1.0f, 0.0f},
        .fovy       = 45,
        .projection = CAMERA_PERSPECTIVE,
    };

    Cube cube = {
        .pos     = {0.0f, 0.0f, 0.0f},
        .forward = {0.0, 0.0, -1.0f},
        .up      = {0.0f, 1.0f, 0.0f},
        .size    = {1.0f, 1.0f, 1.0f},
        .color   = RED,
    };

    State state = STATE_INTRO;
    // State state = STATE_CALIBRATE_KEY_MOUSE;

    Matrix target_cubes[NUM_TARGET_CUBES] = {0};
    for (int i = 0; i < NUM_TARGET_CUBES; i++)
        target_cubes[i] = default_cubes[i];

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HCI User Study - Keyboard + Mouse vs. Gamepad Controller");
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        state = handle_state_change(state);

        /* update */
        switch(state) {
        case STATE_INTRO:
            break;
        case STATE_CALIBRATE_KEY_MOUSE:
            break;
        case STATE_CALIBRATE_GAMEPAD:
            break;
        case STATE_MOVEMENT_KEY_MOUSE:
        case STATE_MOVEMENT_GAMEPAD:
            movement_state_update(state, &cube);
            break;
        case STATE_END:
        default:
            break;
        }

        /* drawing */
        BeginDrawing();
            ClearBackground(RAYWHITE);
            switch(state) {
            case STATE_INTRO:
                render_intro();
                break;
            case STATE_CALIBRATE_KEY_MOUSE:
                render_key_mouse_calibrate();
                break;
            case STATE_CALIBRATE_GAMEPAD:
                break;
            case STATE_MOVEMENT_KEY_MOUSE:
            case STATE_MOVEMENT_GAMEPAD:
                BeginMode3D(camera);
                    movement_state_render(cube, target_cubes);
                EndMode3D();
                break;
            case STATE_END:
            default:
                break;
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
