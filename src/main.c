#include "raylib.h"
#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>

#define FACTOR 100
#define SCREEN_WIDTH  (16 * FACTOR)
#define SCREEN_HEIGHT (9  * FACTOR)
#define TICK_HEIGHT 25
#define TICK_SPACING 100
#define ASPECT ((float)SCREEN_WIDTH / SCREEN_HEIGHT)
#define RADIUS 5

#define ARRAY_LEN(array) (sizeof(array)/sizeof(array[0]))
#define MAX_COEFFICIENTS 10
#define DATA_COUNT 41

void draw_grid()
{
    DrawLine(0, SCREEN_HEIGHT / 2.0f, SCREEN_WIDTH, SCREEN_HEIGHT / 2.0f, BLACK);
    DrawLine(SCREEN_WIDTH / 2.0f, 0, SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT, BLACK);
    for (size_t i = 0; i * TICK_SPACING < SCREEN_HEIGHT / 2.0f; i++) {
        DrawLine(SCREEN_WIDTH / 2.0f - TICK_HEIGHT / 2.0f, SCREEN_HEIGHT / 2.0f + i * TICK_SPACING,
                 SCREEN_WIDTH / 2.0f + TICK_HEIGHT / 2.0f, SCREEN_HEIGHT / 2.0f + i * TICK_SPACING, BLACK);
        DrawLine(SCREEN_WIDTH / 2.0f - TICK_HEIGHT / 2.0f, SCREEN_HEIGHT / 2.0f - i * TICK_SPACING,
                 SCREEN_WIDTH / 2.0f + TICK_HEIGHT / 2.0f, SCREEN_HEIGHT / 2.0f - i * TICK_SPACING, BLACK);
    }
    for (size_t i = 0; i * TICK_SPACING < SCREEN_WIDTH / 2.0f; i++) {
        DrawLine(SCREEN_WIDTH / 2.0f + i * TICK_SPACING, SCREEN_HEIGHT / 2.0f - TICK_HEIGHT / 2.0f,
                 SCREEN_WIDTH / 2.0f + i * TICK_SPACING, SCREEN_HEIGHT / 2.0f + TICK_HEIGHT / 2.0f, BLACK);
        DrawLine(SCREEN_WIDTH / 2.0f - i * TICK_SPACING, SCREEN_HEIGHT / 2.0f - TICK_HEIGHT / 2.0f,
                 SCREEN_WIDTH / 2.0f - i * TICK_SPACING, SCREEN_HEIGHT / 2.0f + TICK_HEIGHT / 2.0f, BLACK);
    }
}

Vector2 convert_pos(float x, float y, float zoom)
{
    (void)zoom;
    return (Vector2){
        ( x * TICK_SPACING + SCREEN_WIDTH  / 2.0f),
        (-y * TICK_SPACING + SCREEN_HEIGHT / 2.0f),
    };
}

float calc_taylor(float x, float *coeffs, size_t coeff_count)
{
    float sum = 0.0f;
    for (size_t i = 0; i < coeff_count; i++) {
        float x_term = 1.0;
        for (size_t j = 0; j < i; j++)
            x_term *= x;
        sum += coeffs[i] * x_term;
    }

    return sum;
}

void calc_graph(float start, float step, Vector2 *graph, size_t data_count, float *coeffs, size_t coeff_count)
{
    size_t data_idx = 0;
    for (float x = start; data_idx < data_count; x += step, data_idx++) {
        graph[data_idx].x = x;
        graph[data_idx].y = calc_taylor(x, coeffs, coeff_count);
    }
}

void calc_sin_graph(float start, float step, Vector2 *graph, size_t data_count)
{
    size_t data_idx = 0;
    for (float x = start; data_idx < data_count; x += step, data_idx++) {
        graph[data_idx].x = x;
        graph[data_idx].y = 3 * sinf(x * 1);
    }
}

float cost(Vector2 *data_set, size_t data_count, float *gradients, size_t gradient_count)
{
    float result = 0.0;
    for (size_t i = 0; i < data_count; i++) {
        float y = calc_taylor(data_set[i].x, gradients, gradient_count);
        float d = y - data_set[i].y;
        result += d * d;
    }
    return result / data_count;
}


int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gradient Descent");

    // srand(0);
    // srand(42);
    srand(time(0));

    // float coeffs[] = {
    //     0.5,
    //     0.0,
    //     -0.4,
    //     0.0,
    //     0.1,
    // };
    // float coeffs[] = {
    //     1.5,
    //     1.0,
    //     0.4,
    //     -1.0,
    //     0.1,
    // };
    float coeffs[] = {
        1.0,
        0.0,
        0.0,
        0.0,
    };

    size_t num_coeffs = ARRAY_LEN(coeffs);
    assert(num_coeffs < MAX_COEFFICIENTS);

    /* leave the first coefficient as zero */
    float guess_coeffs[MAX_COEFFICIENTS] = {0};
    for (size_t i = 1; i < num_coeffs; i++) {
        float random = ((float)rand() / RAND_MAX)*2.0f - 1.0;
        guess_coeffs[i] = random;
    }
    float gradients[MAX_COEFFICIENTS];

    printf("coefficients\n");
    printf("------------\n");
    for (size_t i = 0; i < num_coeffs; i++) {
        printf("a%zu = %f\n", i, coeffs[i]);
    }
    printf("\n");

    printf("guess coefficients\n");
    printf("------------\n");
    for (size_t i = 0; i < num_coeffs; i++) {
        printf("a%zu = %f\n", i, guess_coeffs[i]);
    }
    printf("\n");

    Vector2 *graph = malloc(DATA_COUNT * sizeof(Vector2));
    float step = 0.1;
    float start = -2.0;
    calc_graph(start, step, graph, DATA_COUNT, coeffs, num_coeffs);
    // calc_sin_graph(start, step, graph, DATA_COUNT);
    Vector2 *guess_graph = malloc(DATA_COUNT * sizeof(Vector2));
    calc_graph(start, step, guess_graph, DATA_COUNT, guess_coeffs, num_coeffs);

    float eps  = 1e-6;
    float rate = 1e-3;

    while(!WindowShouldClose()) {
        // if (IsKeyPressed(KEY_SPACE)) {
        if (IsKeyDown(KEY_SPACE) || IsKeyPressed(KEY_S)) {
            float c = cost(graph, DATA_COUNT, guess_coeffs, num_coeffs);
            for (size_t i = 0; i < num_coeffs; i++) {
                float saved = guess_coeffs[i];
                guess_coeffs[i] += eps;
                float g = (cost(graph, DATA_COUNT, guess_coeffs, num_coeffs) - c) / eps;
                gradients[i] = guess_coeffs[i] - rate * g;
                guess_coeffs[i] = saved;
            }
            for (size_t i = 0; i < num_coeffs; i++) {
                guess_coeffs[i] = gradients[i];
            }
            calc_graph(start, step, guess_graph, DATA_COUNT, guess_coeffs, num_coeffs);
            printf("cost %f\n", c);
            printf("guess coefficients\n");
            printf("------------\n");
            for (size_t i = 0; i < num_coeffs; i++) {
                printf("a%zu = %f\n", i, guess_coeffs[i]);
            }
            printf("\n");
        }

        float zoom = 2.0f;
        BeginDrawing();
            ClearBackground(RAYWHITE);
            draw_grid();
            for (size_t i = 0; i < DATA_COUNT; i++) {
                DrawCircleV(convert_pos(graph[i].x, graph[i].y, zoom), RADIUS, RED);
                DrawCircleV(convert_pos(guess_graph[i].x, guess_graph[i].y, zoom), RADIUS, BLUE);
            }
        EndDrawing();
    }
    CloseWindow();

    free(graph);
    return 0;
}
