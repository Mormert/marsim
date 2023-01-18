cmake_minimum_required(VERSION 3.24)

project(marsim)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(box2d)

option(BOX2D_BUILD_UNIT_TESTS OFF)
option(BOX2D_BUILD_TESTBED OFF)

set (MARSIM_SOURCE_FILES
        src/framework/draw.cpp
        src/framework/draw.h
        src/framework/imgui_impl_glfw.cpp
        src/framework/imgui_impl_glfw.h
        src/framework/imgui_impl_opengl3.cpp
        src/framework/imgui_impl_opengl3.h
        src/framework/settings.h
        src/framework/settings.cpp
        src/framework/application.cpp
		src/main.cpp
		src/simulation.cpp
		src/wheel.cpp
		src/robot.cpp
		3rdparty/stb_image.cpp
		src/terrain.cpp)

add_executable(marsim ${MARSIM_SOURCE_FILES})
target_include_directories(marsim PRIVATE src math 3rdparty)
target_link_libraries(marsim PUBLIC box2d glfw imgui sajson glad)

FILE(COPY src/data DESTINATION ${PROJECT_BINARY_DIR})
