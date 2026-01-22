#include "test.h"
#include "psx_svg_node.h"
#include "psx_svg_render.h"

PERF_TEST_DEFINE(SvgParser);

// Complex SVG Tiny 1.2 test sample with multiple features
static const char* complex_svg_tiny_12 =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" "
    "    xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
    "    version=\"1.2\" baseProfile=\"tiny\" "
    "    width=\"800\" height=\"600\" viewBox=\"0 0 800 600\">"

    "<defs>"
    // Complex gradients
    "<linearGradient id=\"grad1\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\" gradientUnits=\"objectBoundingBox\">"
    "<stop offset=\"0%\" stop-color=\"#ff0000\" stop-opacity=\"1\"/>"
    "<stop offset=\"50%\" stop-color=\"#00ff00\" stop-opacity=\"0.8\"/>"
    "<stop offset=\"100%\" stop-color=\"#0000ff\" stop-opacity=\"0.6\"/>"
    "</linearGradient>"

    "<radialGradient id=\"grad2\" cx=\"50%\" cy=\"50%\" r=\"50%\" gradientUnits=\"objectBoundingBox\">"
    "<stop offset=\"0%\" stop-color=\"yellow\" stop-opacity=\"1\"/>"
    "<stop offset=\"100%\" stop-color=\"orange\" stop-opacity=\"0.5\"/>"
    "</radialGradient>"

    // Solid color
    "<solidColor xml:id=\"solidBlue\" solid-color=\"blue\" solid-opacity=\"0.7\"/>"

    // Reusable group
    "<g id=\"reusableGroup\">"
    "<rect x=\"0\" y=\"0\" width=\"50\" height=\"50\" fill=\"red\"/>"
    "<circle cx=\"25\" cy=\"25\" r=\"20\" fill=\"yellow\"/>"
    "</g>"
    "</defs>"

    // Complex shapes with transforms
    "<g transform=\"translate(100, 100) rotate(45) scale(1.5)\">"
    "<rect x=\"0\" y=\"0\" width=\"100\" height=\"80\" fill=\"url(#grad1)\" "
    "stroke=\"black\" stroke-width=\"2\" stroke-dasharray=\"5,3\"/>"
    "<circle cx=\"150\" cy=\"50\" r=\"40\" fill=\"url(#grad2)\"/>"
    "</g>"

    // Complex path with curves
    "<path d=\"M 200,200 C 200,100 400,100 400,200 S 600,300 600,200 "
    "Q 650,250 700,200 T 800,200 L 800,400 Z\" "
    "fill=\"url(#solidBlue)\" stroke=\"black\" stroke-width=\"3\"/>"

    // Multiple polylines and polygons
    "<polyline points=\"50,350 100,300 150,320 200,280 250,310 300,290\" "
    "fill=\"none\" stroke=\"green\" stroke-width=\"4\"/>"

    "<polygon points=\"400,350 450,300 500,350 480,400 420,400\" "
    "fill=\"purple\" stroke=\"black\" stroke-width=\"2\"/>"

    // Text elements
    "<text x=\"100\" y=\"500\" font-size=\"24\" font-family=\"Arial\" fill=\"black\">"
    "Complex SVG Tiny 1.2 Test"
    "</text>"

    "<text x=\"100\" y=\"530\" font-size=\"18\" fill=\"gray\">"
    "<tspan x=\"100\" dy=\"0\">Multiple features:</tspan>"
    "<tspan x=\"100\" dy=\"20\">Gradients, Transforms, Paths</tspan>"
    "</text>"

    // Use elements for reuse
    "<use xlink:href=\"#reusableGroup\" x=\"500\" y=\"400\" transform=\"scale(2)\"/>"
    "<use xlink:href=\"#reusableGroup\" x=\"600\" y=\"450\" transform=\"rotate(90)\"/>"

    // Animation elements
    "<rect x=\"50\" y=\"50\" width=\"50\" height=\"50\" fill=\"red\">"
    "<animate attributeName=\"x\" from=\"50\" to=\"200\" dur=\"2s\" repeatCount=\"indefinite\"/>"
    "<animate attributeName=\"fill\" values=\"red;green;blue;red\" dur=\"3s\" repeatCount=\"indefinite\"/>"
    "</rect>"

    "</svg>";

// Test 1: SVG data loading performance
PERF_TEST_RUN(SvgParser, LoadComplexSvg)
{
    auto result = RunBenchmark(SvgParser_LoadComplexSvg, [&]() {
        for (int i = 0; i < 2000; i++) {
            psx_svg_node* root = psx_svg_load_data(complex_svg_tiny_12, (uint32_t)strlen(complex_svg_tiny_12));
            if (root) {
                psx_svg_node_destroy(root);
            }
        }
    });

    CompareToBenchmark(SvgParser_LoadComplexSvg, result);
}

// Test 2: Render list creation performance
PERF_TEST_RUN(SvgParser, CreateRenderList)
{
    // Parse SVG once to get the node tree
    psx_svg_node* root = psx_svg_load_data(complex_svg_tiny_12, (uint32_t)strlen(complex_svg_tiny_12));
    ASSERT_NE(root, nullptr);

    auto result = RunBenchmark(SvgParser_CreateRenderList, [&]() {
        for (int i = 0; i < 2000; i++) {
            psx_svg_render_list* list = psx_svg_render_list_create(root);
            if (list) {
                psx_svg_render_list_destroy(list);
            }
        }
    });

    CompareToBenchmark(SvgParser_CreateRenderList, result);
    psx_svg_node_destroy(root);
}
