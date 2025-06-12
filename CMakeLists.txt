cmake_minimum_required(VERSION 3.10)
project(TorrentProject CXX) # Project name remains TorrentProject

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define project-specific include directories
# This tells the compiler where to find your own header files (e.g., Window.hpp)
# The path is relative to the CMakeLists.txt file
include_directories(${CMAKE_SOURCE_DIR}/inc)

# Find GLFW (ensure you have libglfw3-dev installed on Linux)
find_package(glfw3 CONFIG REQUIRED) # Or find_package(GLFW REQUIRED) if installed globally

# Find OpenGL and GLEW (ensure you have libglew-dev and libgl-dev installed on Linux)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# --- ImGui Integration ---
# ImGui source files (relative to the project root, where vendor/imgui is)
set(IMGUI_SOURCES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)

# Add ImGui's include directories
# These paths are also relative to the project root
include_directories(vendor/imgui vendor/imgui/backends)
# --- End ImGui Integration ---


# Define all your application's source files
# Add all your .cpp files from src/ here
# List headers from inc/ for dependency tracking if you want, but they are not compiled directly
set(APP_SOURCES
    src/main.cpp
    src/Window.cpp
    # Add other .cpp files from your src/ folder here, e.g.:
    # src/TorrentClient.cpp
    # src/Tracker.cpp
    # inc/TorrentClient.hpp # You can list headers too
    # inc/Tracker.hpp
)


# Create your executable
# Combine your application sources with the ImGui sources
add_executable(${PROJECT_NAME} # Uses the project name "TorrentProject" for the executable
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
)

# Link necessary libraries
target_link_libraries(${PROJECT_NAME}
    glfw
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

# Optional: Set properties for macOS bundles (if needed for deployment)
if(APPLE)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE TRUE)
endif()