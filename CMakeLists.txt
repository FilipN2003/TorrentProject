cmake_minimum_required(VERSION 3.10)
project(TorrentProject CXX) # Project name remains TorrentProject

cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE "GLVND")

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


# --- Libtorrent-Rasterbar Integration ---
# Libtorrent-Rasterbar depends on Boost and OpenSSL.
# It's good practice to find these explicitly before libtorrent itself.

# Find Boost libraries
# libtorrent-rasterbar commonly uses system, thread, date_time, and chrono components.
find_package(Boost COMPONENTS system thread date_time chrono REQUIRED) # Adjust version as needed
if (Boost_FOUND)
    message(STATUS "Found Boost: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost library not found. Please install Boost development files (e.g., libboost-dev).")
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL development files (e.g., libssl-dev).")
endif()

# Find Libtorrent-Rasterbar itself
# It typically installs a Config.cmake file, so find_package should work.
find_package(LibtorrentRasterbar REQUIRED)
if (LibtorrentRasterbar_FOUND)
    message(STATUS "Found Libtorrent-Rasterbar: ${LibtorrentRasterbar_LIBRARIES}")
    include_directories(${LibtorrentRasterbar_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Libtorrent-Rasterbar not found. Please install libtorrent-rasterbar-dev.")
endif()
# --- End Libtorrent-Rasterbar Integration ---


# Define all your application's source files
# Add all your .cpp files from src/ here
# List headers from inc/ for dependency tracking if you want, but they are not compiled directly
set(APP_SOURCES
    src/main.cpp
    src/Window.cpp
    # Add other .cpp files from your src/ folder here, e.g.:
    # src/TorrentClient.cpp
    # src/Tracker.cpp
    # src/YourLibtorrentUsage.cpp # Example file where you'd use libtorrent
    # inc/TorrentClient.hpp
    # inc/Tracker.hpp
)


# Create your executable
# Combine your application sources with the ImGui sources
add_executable(${PROJECT_NAME} # Uses the project name "TorrentProject" for the executable
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
)

# Link necessary libraries
# Order can sometimes matter, typically more dependent libraries first.
target_link_libraries(${PROJECT_NAME}
    glfw
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${LibtorrentRasterbar_LIBRARIES} # Link libtorrent-rasterbar
    ${Boost_LIBRARIES}               # Link Boost components
    ${OPENSSL_LIBRARIES}             # Link OpenSSL
)

# Optional: Set properties for macOS bundles (if needed for deployment)
if(APPLE)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE TRUE)
endif()