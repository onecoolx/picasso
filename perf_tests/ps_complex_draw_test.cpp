
#include "test.h"

PERF_TEST_DEFINE(Complex);

PERF_TEST_RUN(Complex, CirclesDrawing)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    // create 500 circles
    ps_path* circles[500];
    for (int i = 0; i < 500; i++) {
        circles[i] = ps_path_create();
        float radius = 5.0f + (i % 50) * 0.5f; // radius 5-30px
        ps_point center = {i % 100 * 2.0f, (i / 100) * 20.0f};
        ps_rect rc = {center.x - radius, center.y - radius, radius * 2, radius * 2};
        ps_path_add_ellipse(circles[i], &rc);
    }

    // fill color
    ps_color fill_color = {1.0f, 0.2f, 0.3f, 1.0f};
    ps_set_source_color(ctx, &fill_color);

    auto result = RunBenchmark(Complex_CirclesDrawing, [&]() {
        for (int i = 0; i < 500; i++) {
            ps_set_path(ctx, circles[i]);
            ps_fill(ctx);
        }
    }, 10);

    CompareToBenchmark(Complex_CirclesDrawing, result);

    // clean
    for (int i = 0; i < 500; i++) {
        ps_path_unref(circles[i]);
    }
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Complex, RectanglesDrawing)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    // create 500 rectangles
    ps_path* rectangles[1000];
    for (int i = 0; i < 1000; i++) {
        rectangles[i] = ps_path_create();
        float width = 10.0f + (i % 40) * 1.0f; // width 10-50px
        float height = 10.0f + (i % 30) * 1.0f; // height10-40px
        ps_rect rect = {i % 100 * 2.0f, (i / 100) * 20.0f, width, height};
        ps_path_add_rect(rectangles[i], &rect);
    }

    // create 500 colors
    ps_color colors[1000];
    for (int i = 0; i < 1000; i++) {
        colors[i] = {
            (float)(i % 255) / 255.0f, // Red
            (float)((i * 2) % 255) / 255.0f, // Green
            (float)((i * 3) % 255) / 255.0f, // Blue
            1.0f // Alpha
        };
    }

    auto result = RunBenchmark(Complex_RectanglesDrawing, [&]() {
        for (int j = 0; j < 1000; j++) {
            ps_set_source_color(ctx, &colors[j]);
            ps_set_path(ctx, rectangles[j]);
            ps_fill(ctx);
        }
    }, 10);

    CompareToBenchmark(Complex_RectanglesDrawing, result);

    // clean
    for (int i = 0; i < 1000; i++) {
        ps_path_unref(rectangles[i]);
    }
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Complex, GradientShapes)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    // Create 200 random geometric shapes
    ps_path* shapes[200];
    srand(42); // Fixed seed for reproducibility

    for (int i = 0; i < 200; i++) {
        shapes[i] = ps_path_create();
        int shape_type = rand() % 4; // 0:rectangle, 1:circle, 2:ellipse, 3:polygon

        switch (shape_type) {
            case 0: { // Rectangle
                    ps_rect rect = {(float)(rand() % 200), (float)(rand() % 150), 20.0f + rand() % 30, 20.0f + rand() % 30};
                    ps_path_add_rect(shapes[i], &rect);
                    break;
                }
            case 1: { // Circle
                    float radius = 10 + rand() % 20;
                    ps_point center = {(float)(rand() % 200), (float)(rand() % 150)};
                    ps_rect rc = { center.x - radius, center.y - radius, radius * 2, radius * 2 };
                    ps_path_add_ellipse(shapes[i], &rc);
                    break;
                }
            case 2: { // Ellipse
                    ps_rect rect = {(float)(rand() % 200), (float)(rand() % 150), 20.0f + rand() % 40, 15.0f + rand() % 30};
                    ps_path_add_ellipse(shapes[i], &rect);
                    break;
                }
            case 3: { // Polygon
                    ps_point center = {(float)(rand() % 200), (float)(rand() % 150)};
                    int sides = 3 + rand() % 5; // 3-7 sides
                    float radius = 10 + rand() % 20;
                    for (int j = 0; j < sides; j++) {
                        float angle = j * 2.0f * 3.14159f / sides;
                        ps_point p = {center.x + cosf(angle)* radius, center.y + sinf(angle)* radius};
                        if (j == 0) {
                            ps_path_move_to(shapes[i], &p);
                        } else {
                            ps_path_line_to(shapes[i], &p);
                        }
                    }
                    ps_path_sub_close(shapes[i]);
                    break;
                }
        }
    }

    // Create 5 different gradients
    ps_gradient* gradients[5];
    // Linear gradient: spread, start point, end point
    ps_point start1 = {0, 0};
    ps_point end1 = {100, 100};
    gradients[0] = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start1, &end1);

    // Radial gradient: spread, start point, start radius, end point, end radius
    ps_point start2 = {50, 50};
    ps_point end2 = {50, 50};
    gradients[1] = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start2, 10.0f, &end2, 40.0f);

    // Diagonal linear gradient
    ps_point start3 = {0, 100};
    ps_point end3 = {100, 0};
    gradients[2] = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start3, &end3);

    // Conic gradient: spread, origin point, start angle
    ps_point origin = {50, 50};
    gradients[3] = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &origin, 0.0f);

    // Off-center radial gradient
    ps_point start4 = {25, 25};
    ps_point end4 = {75, 75};
    gradients[4] = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start4, 5.0f, &end4, 50.0f);

    // Add color stops to each gradient
    ps_color colors[][2] = {
        {{1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}, // Red to blue
        {{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}}, // Yellow to purple
        {{0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}}, // Green to cyan
        {{1.0f, 0.5f, 0.0f, 1.0f}, {0.5f, 0.0f, 1.0f, 1.0f}}, // Orange to indigo
        {{0.5f, 0.0f, 0.5f, 1.0f}, {0.0f, 0.5f, 0.5f, 1.0f}} // Purple to teal
    };

    for (int i = 0; i < 5; i++) {
        ps_gradient_add_color_stop(gradients[i], 0.0f, &colors[i][0]);
        ps_gradient_add_color_stop(gradients[i], 1.0f, &colors[i][1]);
    }

    auto result = RunBenchmark(Complex_GradientShapes, [&]() {
        for (int j = 0; j < 200; j++) {
            int gradient_index = j % 5;
            ps_set_source_gradient(ctx, gradients[gradient_index]);
            ps_set_path(ctx, shapes[j]);
            ps_fill(ctx);
        }
    }, 10);

    CompareToBenchmark(Complex_GradientShapes, result);

    // Clean up resources
    for (int i = 0; i < 200; i++) {
        ps_path_unref(shapes[i]);
    }
    for (int i = 0; i < 5; i++) {
        ps_gradient_unref(gradients[i]);
    }
    ps_context_unref(ctx);
}
