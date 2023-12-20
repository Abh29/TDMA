cmake_minimum_required(VERSION 3.9)
project(playground)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra -g)

find_package(OpenMP REQUIRED)

set(CMAKE_CXX_FLAGS " -lEGL -lGLU -lOpenGL ")

add_link_options( -lSDL2 -lGL -lSDL2_image )
add_executable(task1 tdma_1d.cpp src/imgui.cpp src/imgui_impl_opengl3.cpp src/imgui_draw.cpp
        src/imgui_tables.cpp src/imgui_widgets.cpp src/imgui_impl_sdl2.cpp src/imgui_demo.cpp)

add_executable(task2 tdma_2d.cpp src/imgui.cpp src/imgui_impl_opengl3.cpp src/imgui_draw.cpp
        src/imgui_tables.cpp src/imgui_widgets.cpp src/imgui_impl_sdl2.cpp src/imgui_demo.cpp)

add_executable(task2_mpi tdma_2d_mpi.cpp src/imgui.cpp src/imgui_impl_opengl3.cpp src/imgui_draw.cpp
        src/imgui_tables.cpp src/imgui_widgets.cpp src/imgui_impl_sdl2.cpp src/imgui_demo.cpp)

if (OpenMP_CXX_FOUND)
    target_link_libraries(task2 PUBLIC OpenMP::OpenMP_CXX)
endif()

find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")
target_link_libraries(task2_mpi PUBLIC MPI::MPI_CXX)