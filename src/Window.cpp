#include "include/Window.hpp"

#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title), m_window(nullptr), m_imguiContext(nullptr) {
    glfwSetErrorCallback(GlfwErrorCallback);

    if(!InitGLFW()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    if(!InitOpenGL()) {
        throw std::runtime_error("Failed to initialize OpenGL context");
    }

    InitImGui();
}

Window::~Window() {
    CleanupImGui();
    CleanupGLFW();
}

bool Window::InitGLFW() {
    if(!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(m_window, GlfwFramebufferSizeCallback);

    return true;
}

bool Window::InitOpenGL() {
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        return false;
    }

    if(!GLEW_VERSION_3_3) {
        std::cerr << "OpenGL 3.3 not supported!" << std::endl; //OVDE OBRATITI PAZNJU JER TO MOZDA NIJE VERZIJA SA KOJOM MI RADIMO
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Window::InitImGui() {
    m_imguiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io; //Da izbegnemo warning za neiskoriscenu promenljivu

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //PRETHODNA DVA REDA SAM ZAKOMENTARISAO JER IZ NEKOG RAZLOGA PROGRAM NE VIDI GDE SE OVE PROMENJLIVE NALAZE I CRVENI
    //TAKODJE ZBOG TOGA SAM MORAO DA ZAKOMENTARISEM JEDAN DEO U EndImGuiFrame

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    //Mislim da sam kod objasnjava zasto je prethodno zakomentarisano

    /*ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }*/
    //Prethodni blok zakomentarisan opet zbog ovih flagova koji mi nisu radili

    ImGui_ImplGlfw_InitForOpenGL(m_window, true); 
    ImGui_ImplOpenGL3_Init("#version 130"); //Obratiti paznju ovde na verziju
}

void Window::CleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(m_imguiContext); // Pass the context pointer to destroy
    m_imguiContext = nullptr; // Clear the pointer
}

void Window::CleanupGLFW() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::BeginImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::EndImGuiFrame() {
    ImGui::Render(); // Generate draw data

    // Get framebuffer size to ensure correct rendering to the backbuffer
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Clear the screen (you might do this before ImGui::Render() if you have other scene rendering)
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f); // A pleasant blue-grey
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render ImGui draw data
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Handle ImGui viewports (if enabled)
    /*ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }*/

    // Swap buffers to display the rendered frame
    glfwSwapBuffers(m_window);
}