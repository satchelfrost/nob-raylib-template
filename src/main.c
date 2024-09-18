#include "raylib.h"
#include "rcamera.h"
#include "rlgl.h"
#include "raymath.h"
#include <assert.h>

#define SCREEN_FACTOR 80
#define SCREEN_WIDTH  (16 * SCREEN_FACTOR)
#define SCREEN_HEIGHT ( 9 * SCREEN_FACTOR)

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

typedef struct {
    Vector3 pos;
    Vector3 forward;
    Vector3 up;
    Vector3 size;
    Color color;
    Matrix m;
} Cube;

typedef struct {
    Vector3 pos;
    Color color;
    float radius;
} Sphere;

typedef enum {
    STATE_INTRO,    
    STATE_CALIBRATE_KEY_MOUSE,    
    STATE_CALIBRATE_GAMEPAD,    
    STATE_TEST_KEY_MOUSE,    
    STATE_TEST_GAMEPAD,    
    STATE_END,
} State;

void test_update(State state)
{
    if (state == STATE_TEST_KEY_MOUSE)       update_cube_key_mouse(&cube);
    else if (state == STATE_TEST_KEY_MOUSE)  update_cube_gamepad(&cube);
    else return;

    if (IsKeyPressed(KEY_SPACE)) {
        if (target_cube_count + 1 <= NUM_TARGET_CUBES) target_cubes[target_cube_count++] = cube.m;
        else TraceLog(LOG_INFO, "max cubes reached");
    }
    if (IsKeyPressed(KEY_P)) {
        for (int i = 0; i < target_cube_count; i++) {
            Matrix m = target_cubes[i];
            TraceLog(LOG_INFO, "{");
            TraceLog(LOG_INFO, "    %.2f, %.2f, %.2f, %.2f,",  m.m0, m.m4, m.m8,  m.m12);
            TraceLog(LOG_INFO, "    %.2f, %.2f, %.2f, %.2f,",  m.m1, m.m5, m.m9,  m.m13);
            TraceLog(LOG_INFO, "    %.2f, %.2f, %.2f, %.2f,",  m.m2, m.m6, m.m10, m.m14);
            TraceLog(LOG_INFO, "    %.2f, %.2f, %.2f, %.2f,",  m.m3, m.m7, m.m11, m.m15);
            TraceLog(LOG_INFO, "},");
        }
    }

    /* bounds check */
    if (cube.pos.x < -BOUNDS || cube.pos.x > BOUNDS ||
        cube.pos.y < -BOUNDS || cube.pos.y > BOUNDS ||
        cube.pos.z < -BOUNDS || cube.pos.z > BOUNDS)
        cube.pos = Vector3Zero();

    /* update  cube matrix */
    Matrix r = MatrixInvert(MatrixLookAt((Vector3){0.0f, 0.0f, 0.0f}, cube.forward, cube.up));
    Matrix t = MatrixTranslate(cube.pos.x, cube.pos.y, cube.pos.z);
    Matrix s = MatrixScale(cube.size.x, cube.size.y, cube.size.z);
    cube.m = MatrixMultiply(s, MatrixMultiply(r, t));
}

void test_render(State state)
{
    if (state == STATE_TEST_KEY_MOUSE)       update_cube_key_mouse(&cube);
    else if (state == STATE_TEST_KEY_MOUSE)  update_cube_gamepad(&cube);
    else return;

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

    int target_cube_count = 0;
    Matrix target_cubes[NUM_TARGET_CUBES] = {0};
    for (int i = 0; i < NUM_TARGET_CUBES; i++)
        target_cubes[target_cube_count++] = default_cubes[i];
    target_cube_count = NUM_TARGET_CUBES;

    Sphere sphere = {
        .pos = {0.0f, 0.0f, 0.0f},
        .color = MAGENTA,
        .radius = 0.1f,
    };

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HCI User Study - Keyboard + Mouse vs. Gamepad Controller");

    while(!WindowShouldClose()) {
        /* input */

        /* drawing */
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                DrawGrid(20, 1);
                /* target cubes */
                for (int i = 0; i < target_cube_count; i++) {
                    rlPushMatrix();
                        rlMultMatrixf(MatrixToFloatV(target_cubes[i]).v);
                        DrawCubeWiresV((Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f}, BLACK);
                        DrawLine3D((Vector3){0.0f, 0.0f, -0.5f}, (Vector3){0.0f, 0.0f, -2.0f}, BLACK);
                        rlTranslatef(0.0f, 0.0f, -2.0f);
                        DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, sphere.radius, sphere.color);
                    rlPopMatrix();
                }

                /* user controlled cube */
                rlPushMatrix();
                    rlMultMatrixf(MatrixToFloatV(cube.m).v);
                    DrawCubeV((Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f}, cube.color);
                    DrawLine3D((Vector3){0.0f, 0.0f, -0.5f}, (Vector3){0.0f, 0.0f, -2.0f}, BLACK);
                    rlTranslatef(0.0f, 0.0f, -2.0f);
                    DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, sphere.radius, sphere.color);
                rlPopMatrix();
            EndMode3D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
