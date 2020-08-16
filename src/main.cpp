#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <main_window.h>
#include <style.h>
#include <icons.h>
#include <version.h>
#include <spdlog/spdlog.h>
#include <bandplan.h>
#include <module.h>
#include <stb_image.h>
#include <config.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#ifdef _WIN32
#include <Windows.h>
#endif

static void glfw_error_callback(int error, const char* description) {
    spdlog::error("Glfw Error {0}: {1}", error, description);
}

int main() {
#ifdef _WIN32
    //FreeConsole();
#endif

    spdlog::info("SDR++ v" VERSION_STR);

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "SDR++ v" VERSION_STR " (Built at " __TIME__ ", " __DATE__ ")", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Load app icon
    GLFWimage icons[10];
    icons[0].pixels = stbi_load("res/icons/sdrpp.png", &icons[0].width, &icons[0].height, 0, 4);
    icons[1].pixels = (unsigned char*)malloc(16 * 16 * 4); icons[1].width = icons[1].height = 16;
    icons[2].pixels = (unsigned char*)malloc(24 * 24 * 4); icons[2].width = icons[2].height = 24;
    icons[3].pixels = (unsigned char*)malloc(32 * 32 * 4); icons[3].width = icons[3].height = 32;
    icons[4].pixels = (unsigned char*)malloc(48 * 48 * 4); icons[4].width = icons[4].height = 48;
    icons[5].pixels = (unsigned char*)malloc(64 * 64 * 4); icons[5].width = icons[5].height = 64;
    icons[6].pixels = (unsigned char*)malloc(96 * 96 * 4); icons[6].width = icons[6].height = 96;
    icons[7].pixels = (unsigned char*)malloc(128 * 128 * 4); icons[7].width = icons[7].height = 128;
    icons[8].pixels = (unsigned char*)malloc(196 * 196 * 4); icons[8].width = icons[8].height = 196;
    icons[9].pixels = (unsigned char*)malloc(256 * 256 * 4); icons[9].width = icons[9].height = 256;
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[1].pixels, 16, 16, 16 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[2].pixels, 24, 24, 24 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[3].pixels, 32, 32, 32 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[4].pixels, 48, 48, 48 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[5].pixels, 64, 64, 64 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[6].pixels, 96, 96, 96 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[7].pixels, 128, 128, 128 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[8].pixels, 196, 196, 196 * 4, 4);
    stbir_resize_uint8(icons[0].pixels, icons[0].width, icons[0].height, icons[0].width * 4, icons[9].pixels, 256, 256, 256 * 4, 4);
    glfwSetWindowIcon(window, 10, icons);
    stbi_image_free(icons[0].pixels);
    for (int i = 1; i < 10; i++) {
        free(icons[i].pixels);
    }

    if (glewInit() != GLEW_OK) {
        spdlog::error("Failed to initialize OpenGL loader!");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    // Load config
    spdlog::info("Loading config");
    config::load("config.json");
    config::startAutoSave();

    style::setDefaultStyle();

    spdlog::info("Loading icons");
    icons::load();

    spdlog::info("Loading band plans");
    bandplan::loadFromDir("bandplans");

    spdlog::info("Loading band plans color table");
    bandplan::loadColorTable("band_colors.json");

    windowInit();

    spdlog::info("Ready.");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int wwidth, wheight;
        glfwGetWindowSize(window, &wwidth, &wheight);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(wwidth, wheight));
        
        drawWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0666f, 0.0666f, 0.0666f, 1.0f);
        //glClearColor(0.90f, 0.90f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}