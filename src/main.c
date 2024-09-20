#include "raylib.h"
#include "rcamera.h"
#include "rlgl.h"
#include "raymath.h"
#include <assert.h>

#define SCREEN_FACTOR 80
#define SCREEN_WIDTH  (16 * SCREEN_FACTOR)
#define SCREEN_HEIGHT ( 9 * SCREEN_FACTOR)
#define FONT_SIZE 22
#define MARGIN 50

#define KEY_SPACING 10 
#define KEY_WIDTH 100
#define PROGRESS_BAR_TIC_LEN 50
#define PROGRESS_BAR_PADDING 10
#define KM_PROGRESS_BAR_TICS 7
#define GP_PROGRESS_BAR_TICS 10
#define ANALOG_CIRCLE_BOUND 100
#define ANALOG_CIRCLE_TINY 30

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

typedef struct {
    int w;
    int a;
    int s;
    int d;
    int up;
    int down;
    int right_click;
} Key_Mouse_Calibrate;
static Key_Mouse_Calibrate km = {0};

typedef struct {
    int l_joy_up;
    int l_joy_down;
    int l_joy_left;
    int l_joy_right;
    int r_joy_up;
    int r_joy_down;
    int r_joy_left;
    int r_joy_right;
    int l_shoulder;
    int r_shoulder;
} Gamepad_Calibrate;
static Gamepad_Calibrate gp = {0};

void update_cube_gamepad(Cube *cube);
void update_cube_key_mouse(Cube *cube);
float render_text_centered(const char *msg, int y, Color color);

void draw_progress_bar(int full_bar_len, int progress)
{
    int bar_x = SCREEN_WIDTH / 2 - full_bar_len / 2;
    const char *progress_txt = "Calibration progress";
    int txt_width = MeasureText(progress_txt, FONT_SIZE);
    DrawText(progress_txt,
             SCREEN_WIDTH / 2 - txt_width / 2,
             SCREEN_HEIGHT - MARGIN - PROGRESS_BAR_TIC_LEN - 2 * PROGRESS_BAR_PADDING - FONT_SIZE,
             FONT_SIZE, BLACK);
    DrawRectangle(bar_x - PROGRESS_BAR_PADDING,
                  SCREEN_HEIGHT - MARGIN - PROGRESS_BAR_TIC_LEN - PROGRESS_BAR_PADDING,
                  full_bar_len + 2 * PROGRESS_BAR_PADDING,
                  PROGRESS_BAR_TIC_LEN + 2 * PROGRESS_BAR_PADDING,
                  BLACK);
    for (int i = 0; i < progress; i++) {
        DrawRectangle(bar_x,
                      SCREEN_HEIGHT - MARGIN - PROGRESS_BAR_TIC_LEN,
                      PROGRESS_BAR_TIC_LEN,
                      PROGRESS_BAR_TIC_LEN, GREEN);
        bar_x += PROGRESS_BAR_TIC_LEN;
    }
}

int update_gamepad_calibrate()
{
    float left_joy_fb = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
    float left_joy_lr = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
    if (!gp.l_joy_up)    gp.l_joy_up    = (left_joy_fb <= -DEAD_ZONE);
    if (!gp.l_joy_down)  gp.l_joy_down  = (left_joy_lr <= -DEAD_ZONE);
    if (!gp.l_joy_left)  gp.l_joy_left  = (left_joy_fb >=  DEAD_ZONE);
    if (!gp.l_joy_right) gp.l_joy_right = (left_joy_lr >=  DEAD_ZONE);

    float right_joy_fb = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
    float right_joy_lr = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
    if (!gp.r_joy_up)    gp.r_joy_up    = (right_joy_fb <= -DEAD_ZONE);
    if (!gp.r_joy_down)  gp.r_joy_down  = (right_joy_lr <= -DEAD_ZONE);
    if (!gp.r_joy_left)  gp.r_joy_left  = (right_joy_fb >=  DEAD_ZONE);
    if (!gp.r_joy_right) gp.r_joy_right = (right_joy_lr >=  DEAD_ZONE);

    if (!gp.l_shoulder) gp.l_shoulder = IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
    if (!gp.r_shoulder) gp.r_shoulder = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);

    return gp.l_joy_up + gp.l_joy_down + gp.l_joy_left + gp.l_joy_right + gp.r_joy_up +
           gp.r_joy_down + gp.r_joy_left + gp.r_joy_right + gp.l_shoulder + gp.r_shoulder;
}

void render_gamepad_calibrate(int progress)
{
    const char *title = "Please test all gamepad inputs, and understand their use";
    int txt_width = MeasureText(title, FONT_SIZE);
    DrawText(title, SCREEN_WIDTH / 2 - txt_width / 2, MARGIN, FONT_SIZE, BLACK);

    /* analog left */
    DrawCircle(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_BOUND, BLACK);
    DrawCircle(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_TINY, WHITE);
    DrawCircle(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + ANALOG_CIRCLE_BOUND, ANALOG_CIRCLE_TINY, RED);
    DrawCircle(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - ANALOG_CIRCLE_BOUND, ANALOG_CIRCLE_TINY, RED);
    DrawCircle(SCREEN_WIDTH / 4 + ANALOG_CIRCLE_BOUND, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_TINY, RED);
    DrawCircle(SCREEN_WIDTH / 4 - ANALOG_CIRCLE_BOUND, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_TINY, RED);

    /* analog right */
    DrawCircle(SCREEN_WIDTH / 4 + SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_BOUND, BLACK);
    DrawCircle(SCREEN_WIDTH / 4 + SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_TINY, WHITE);
    DrawCircle(SCREEN_WIDTH / 4 + SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + ANALOG_CIRCLE_BOUND, ANALOG_CIRCLE_TINY, RED);
    DrawCircle(SCREEN_WIDTH / 4 + SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - ANALOG_CIRCLE_BOUND, ANALOG_CIRCLE_TINY, RED);
    DrawCircle(SCREEN_WIDTH / 4 + SCREEN_WIDTH / 2 + ANALOG_CIRCLE_BOUND, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_TINY, RED);
    DrawCircle(SCREEN_WIDTH / 4 + SCREEN_WIDTH / 2 - ANALOG_CIRCLE_BOUND, SCREEN_HEIGHT / 2, ANALOG_CIRCLE_TINY, RED);

    /* calibration progress bar */
    draw_progress_bar(GP_PROGRESS_BAR_TICS * PROGRESS_BAR_TIC_LEN, progress);

    /* render exit text */
    if (progress == GP_PROGRESS_BAR_TICS) {
        const char *continue_txt = "Press SPACE to continue";
        txt_width = MeasureText(continue_txt, FONT_SIZE);
        DrawText(continue_txt,
                SCREEN_WIDTH / 2 - txt_width / 2,
                SCREEN_HEIGHT / 2 - FONT_SIZE / 2, FONT_SIZE, BLACK);
    }
}

int update_key_mouse_calibrate()
{
    if (!km.w)           km.w = IsKeyPressed(KEY_W);
    if (!km.a)           km.a = IsKeyPressed(KEY_A);
    if (!km.s)           km.s = IsKeyPressed(KEY_S);
    if (!km.d)           km.d = IsKeyPressed(KEY_D);
    if (!km.up)          km.up = IsKeyPressed(KEY_UP);
    if (!km.down)        km.down = IsKeyPressed(KEY_DOWN);
    if (!km.right_click) km.right_click = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

    return km.w + km.a + km.s + km.d + km.up + km.down + km.right_click;
}

void render_key_mouse_calibrate(int progress)
{
    const char *title = "Please test all keybord + mouse inputs, and understand their use";
    int txt_width = MeasureText(title, FONT_SIZE);
    DrawText(title, SCREEN_WIDTH / 2 - txt_width / 2, MARGIN - 10, FONT_SIZE, BLACK);

    /* wasd */
    int x_offset = SCREEN_WIDTH / 4;
    const char *wasd = "W, A, S, D, for translation";
    txt_width = MeasureText(wasd, FONT_SIZE);
    DrawText(wasd,
             x_offset - txt_width / 2,
             SCREEN_HEIGHT / 2 - FONT_SIZE - KEY_WIDTH / 2 - KEY_SPACING,
             FONT_SIZE, BLACK);
    /* w */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2 - KEY_WIDTH / 2,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2,
                  KEY_WIDTH, KEY_WIDTH, km.w ? GREEN: RED);
    /* a */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2 - KEY_WIDTH / 2 - KEY_WIDTH - KEY_SPACING,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  KEY_WIDTH, KEY_WIDTH, km.a ? GREEN : RED);
    /* s */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2  - KEY_WIDTH / 2,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  KEY_WIDTH, KEY_WIDTH, km.s ? GREEN : RED);
    /* d */
    DrawRectangle(-x_offset + SCREEN_WIDTH / 2  - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2 + KEY_WIDTH + KEY_SPACING,
                  KEY_WIDTH, KEY_WIDTH, km.d ? GREEN : RED);

    /* up/down arrow */
    const char *up_down_txt = "up/down arrows for scaling";
    txt_width = MeasureText(up_down_txt, FONT_SIZE);
    DrawText(up_down_txt,
             SCREEN_WIDTH / 2 - txt_width / 2,
             SCREEN_HEIGHT / 4 - FONT_SIZE - KEY_WIDTH / 2 - KEY_SPACING,
             FONT_SIZE, BLACK);
    DrawRectangle(SCREEN_WIDTH / 2 - KEY_WIDTH  - KEY_SPACING / 2,
                  SCREEN_HEIGHT / 4 - KEY_WIDTH / 2,
                  KEY_WIDTH, KEY_WIDTH, km.up ? GREEN : RED);
    DrawRectangle(SCREEN_WIDTH / 2 - KEY_WIDTH / 2 + KEY_WIDTH / 2 + KEY_SPACING / 2,
                  SCREEN_HEIGHT / 4 - KEY_WIDTH / 2,
                  KEY_WIDTH, KEY_WIDTH, km.down ? GREEN : RED);

    /* mouse right click */
    x_offset += SCREEN_WIDTH / 2;
    const char *mouse_txt = "Right-click + mouse movement for rotation";
    txt_width = MeasureText(mouse_txt, FONT_SIZE);
    DrawText(mouse_txt,
             x_offset - txt_width / 2,
             SCREEN_HEIGHT / 2 - FONT_SIZE - KEY_WIDTH / 2 - KEY_SPACING,
             FONT_SIZE, BLACK);
    DrawRectangle(x_offset - KEY_WIDTH / 2,
                  SCREEN_HEIGHT / 2 - KEY_WIDTH / 2,
                  KEY_WIDTH, KEY_WIDTH, km.right_click ? GREEN : RED);

    /* calibration progress bar */
    draw_progress_bar(KM_PROGRESS_BAR_TICS * PROGRESS_BAR_TIC_LEN, progress);

    /* render exit text */
    if (progress == KM_PROGRESS_BAR_TICS) {
        const char *continue_txt = "Press SPACE to continue";
        txt_width = MeasureText(continue_txt, FONT_SIZE);
        DrawText(continue_txt,
                SCREEN_WIDTH / 2 - txt_width / 2,
                SCREEN_HEIGHT / 2 - FONT_SIZE / 2, FONT_SIZE, BLACK);
    }
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

    int y = MARGIN;
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
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) {
        float scale = (cube->size.x + SCALE_SPEED * dt < MAX_CUBE_SCALE) ? SCALE_SPEED * dt : 0.0f;
        cube->size.x += scale;
        cube->size.y += scale;
        cube->size.z += scale;
    }
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1)) {
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

static Matrix target_cubes[NUM_TARGET_CUBES] = {
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
    // State state = STATE_CALIBRATE_GAMEPAD;
    bool ready_for_transition = true;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HCI User Study - Keyboard + Mouse vs. Gamepad Controller");
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        if (ready_for_transition && IsKeyPressed(KEY_SPACE))
            state = (state + 1) % STATE_COUNT;

        /* update */
        int progress = 0;
        switch(state) {
        case STATE_INTRO:
            break;
        case STATE_CALIBRATE_KEY_MOUSE:
            progress = update_key_mouse_calibrate();
            ready_for_transition = (progress == KM_PROGRESS_BAR_TICS) ? true : false;
            break;
        case STATE_CALIBRATE_GAMEPAD:
            progress = update_gamepad_calibrate();
            ready_for_transition = (progress == GP_PROGRESS_BAR_TICS) ? true : false;
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
                render_key_mouse_calibrate(progress);
                break;
            case STATE_CALIBRATE_GAMEPAD:
                render_gamepad_calibrate(progress);
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
