#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>
#include <memory>

struct GLFWwindow;
struct ImGuiContext;

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool ShouldClose() const;
    void BeginImGuiFrame();
    void EndImGuiFrame();
    GLFWwindow* GetGLFWWindow() const { return m_window; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    GLFWwindow* m_window;
    int m_height;
    int m_width;
    std::string m_title;

    ImGuiContext* m_imguiContext;

    bool InitGLFW();
    bool InitOpenGL();
    void InitImGui();
    void CleanupImGui();
    void CleanupGLFW();

    static void GlfwErrorCallback(int error, const char* description);
    static void GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};
#endif