cmake_minimum_required(VERSION 3.24)
project(playground)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra)

set(CMAKE_CXX_FLAGS " -lEGL -lGLU -lOpenGL ")

add_link_options( -lSDL2 -lGL -lSDL2_image )
add_executable(task1 main.cpp backends/imgui.cpp backends/imgui_impl_opengl3.cpp backends/imgui_draw.cpp
        backends/imgui_tables.cpp backends/imgui_widgets.cpp backends/imgui_impl_sdl2.cpp backends/imgui_demo.cpp)
