#include "raylib.h"
#include "rcamera.h"
#include "rlgl.h"
#include "raymath.h"
#include <assert.h>

#define CAMERA_MOUSE_MOVE_SENSITIVITY 0.005f
#define GAMEPAD_ROT_SENSITIVITY 1.0f
#define CAMERA_ROTATION_SPEED 1.0f
#define CAMERA_MOVE_SPEED 10.0f
#define SCREEN_FACTOR 80
#define SCREEN_WIDTH  (16 * SCREEN_FACTOR)
#define SCREEN_HEIGHT ( 9 * SCREEN_FACTOR)
#define CUBE_SPEED 5.0f

void update_camera_gamepad(Camera *camera)
{
    float dt = GetFrameTime();
    float move_speed = CAMERA_MOVE_SPEED * dt;
    CameraYaw(camera, -GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X)  * dt * GAMEPAD_ROT_SENSITIVITY, false);
    CameraPitch(camera,-GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) * dt * GAMEPAD_ROT_SENSITIVITY, false, false, false);
    if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) <= -0.25f) CameraMoveForward(camera,  move_speed, false);
    if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) <= -0.25f) CameraMoveRight(camera,   -move_speed, false);
    if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >=  0.25f) CameraMoveForward(camera, -move_speed, false);
    if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >=  0.25f) CameraMoveRight(camera,    move_speed, false);
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)) CameraMoveUp(camera,  move_speed);
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2))  CameraMoveUp(camera, -move_speed);
}

typedef struct {
    Vector3 pos;
    Vector3 forward;
    Vector3 up;
    Vector3 size;
    Color color;
} Cube;

typedef struct {
    Vector3 pos;
    Color color;
    float radius;
} Sphere;

int main()
{
    Camera camera = {
        .position   = {0.0f, 3.0f, 5.0f},
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

    Sphere sphere = {
        .pos = {0.0f, 0.0f, 0.0f},
        .color = MAGENTA,
        .radius = 0.1f,
    };

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HCI User Study - Keyboard + Mouse vs. Gamepad Controller");

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        /* update */
        Vector3 forward = Vector3Normalize(cube.forward);
        Vector3 up = Vector3Normalize(cube.up);
        Vector3 right = Vector3CrossProduct(forward, up);
        if (IsKeyDown(KEY_W)) cube.pos = Vector3Add(cube.pos, Vector3Scale(forward,  dt * CUBE_SPEED));
        if (IsKeyDown(KEY_S)) cube.pos = Vector3Add(cube.pos, Vector3Scale(forward, -dt * CUBE_SPEED));
        if (IsKeyDown(KEY_D)) cube.pos = Vector3Add(cube.pos, Vector3Scale(right,    dt * CUBE_SPEED));
        if (IsKeyDown(KEY_A)) cube.pos = Vector3Add(cube.pos, Vector3Scale(right,   -dt * CUBE_SPEED));

        // Matrix m = MatrixIdentity();
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            Vector3 rot_y = Vector3RotateByAxisAngle(forward, up,    -delta.x * CAMERA_MOUSE_MOVE_SENSITIVITY);
            Vector3 rot_x = Vector3RotateByAxisAngle(forward, right, -delta.y * CAMERA_MOUSE_MOVE_SENSITIVITY);
            cube.forward = Vector3Normalize(Vector3Add(rot_y, rot_x));
        }

        /* update sphere pos */
        sphere.pos = Vector3Add(Vector3Scale(forward, 2.0f), cube.pos);

        /* drawing */
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                DrawGrid(100, 1);
                DrawLine3D(cube.pos, sphere.pos, BLACK);
                DrawSphere(sphere.pos, sphere.radius, sphere.color);
                DrawCubeV(cube.pos, cube.size, cube.color);
            EndMode3D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
