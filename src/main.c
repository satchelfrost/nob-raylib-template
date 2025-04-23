#include "raylib.h"
#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define FACTOR 50
#define SCREEN_WIDTH  (10 * FACTOR)
#define SCREEN_HEIGHT (10  * FACTOR)
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
        graph[data_idx].y = sinf(x);
    }
}

float cost(Vector2 *data_set, size_t data_count, float *coeffs, size_t coeff_count)
{
    float result = 0.0;
    for (size_t i = 0; i < data_count; i++) {
        float y = calc_taylor(data_set[i].x, coeffs, coeff_count);
        float d = y - data_set[i].y;
        result += d * d;
    }
    return result / data_count;
}

void print_results(float *guess_coeffs, size_t num_coeffs, float cost)
{
    printf("cost %f\n", cost);
    printf("guess coefficients\n");
    printf("------------\n");
    for (size_t i = 0; i < num_coeffs; i++) {
        printf("a%zu = %f\n", i, guess_coeffs[i]);
    }
    printf("\n");
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
        0.0,
        1.0,
        -1.0,
        0.1,
    };

    size_t num_coeffs = ARRAY_LEN(coeffs);
    assert(num_coeffs < MAX_COEFFICIENTS);

    /* leave the first coefficient as zero */
    float guess_coeffs[MAX_COEFFICIENTS] = {0};
    for (size_t i = 1; i < num_coeffs; i++) {
        float random = ((float)rand() / RAND_MAX)*2.0f - 1.0;
        guess_coeffs[i] = random;
    }

    printf("ground truth coefficients\n");
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
    Vector2 *guess_graph = malloc(DATA_COUNT * sizeof(Vector2));
    calc_graph(start, step, guess_graph, DATA_COUNT, guess_coeffs, num_coeffs);
    float eps  = 1e-6;
    float rate = 1e-3;
    size_t data_point_idx = 0;

    while(!WindowShouldClose()) {
        if (IsKeyPressed(KEY_P)) {
            float c = cost(graph, DATA_COUNT, guess_coeffs, num_coeffs);
            print_results(guess_coeffs, num_coeffs, c);
        }

        if (IsKeyDown(KEY_SPACE) || IsKeyPressed(KEY_S)) { // full batch gradient descent
            float c = cost(graph, DATA_COUNT, guess_coeffs, num_coeffs);
            float tmp_coeffs[num_coeffs];
            float adj_coeffs[num_coeffs];
            for (size_t i = 0; i < num_coeffs; i++) {
                memcpy(tmp_coeffs, guess_coeffs, num_coeffs * sizeof(float));
                tmp_coeffs[i] += eps;
                float g = (cost(graph, DATA_COUNT, tmp_coeffs, num_coeffs) - c) / eps;
                adj_coeffs[i] = guess_coeffs[i] - rate * g;
            }
            for (size_t i = 0; i < num_coeffs; i++) {
                guess_coeffs[i] = adj_coeffs[i];
            }
            calc_graph(start, step, guess_graph, DATA_COUNT, guess_coeffs, num_coeffs);
        }

        if (IsKeyDown(KEY_G)) { // stochastic gradient descent
            float c = cost(graph + data_point_idx, 1, guess_coeffs, num_coeffs);
            float tmp_coeffs[num_coeffs];
            float adj_coeffs[num_coeffs];
            for (size_t i = 0; i < num_coeffs; i++) {
                memcpy(tmp_coeffs, guess_coeffs, num_coeffs * sizeof(float));
                tmp_coeffs[i] += eps;
                float g = (cost(graph + data_point_idx, 1, tmp_coeffs, num_coeffs) - c) / eps;
                adj_coeffs[i] = guess_coeffs[i] - rate * g;
            }
            for (size_t i = 0; i < num_coeffs; i++) {
                guess_coeffs[i] = adj_coeffs[i];
            }
            data_point_idx = (data_point_idx + 1) % DATA_COUNT;
            calc_graph(start, step, guess_graph, DATA_COUNT, guess_coeffs, num_coeffs);
        }

        if (IsKeyDown(KEY_I)) { // stochastic gradient descent over whole dataset
            float tmp_coeffs[num_coeffs];
            float adj_coeffs[num_coeffs];
            for (size_t i = 0; i < DATA_COUNT; i++) {
                float c = cost(graph + i, 1, guess_coeffs, num_coeffs);
                for (size_t j = 0; j < num_coeffs; j++) {
                    memcpy(tmp_coeffs, guess_coeffs, num_coeffs * sizeof(float));
                    tmp_coeffs[j] += eps;
                    float g = (cost(graph + i, 1, tmp_coeffs, num_coeffs) - c) / eps;
                    adj_coeffs[j] = guess_coeffs[j] - rate * g;
                }
                for (size_t j = 0; j < num_coeffs; j++)
                    guess_coeffs[j] = adj_coeffs[j];
                calc_graph(start, step, guess_graph, DATA_COUNT, guess_coeffs, num_coeffs);
            }
        }

        if (IsKeyPressed(KEY_R)) {
            for (size_t i = 1; i < num_coeffs; i++) {
                float random = ((float)rand() / RAND_MAX)*2.0f - 1.0;
                guess_coeffs[i] = random;
            }
            calc_graph(start, step, guess_graph, DATA_COUNT, guess_coeffs, num_coeffs);
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
