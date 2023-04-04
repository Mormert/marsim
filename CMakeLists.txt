cmake_minimum_required(VERSION 3.21)

project(marsim)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rdparty/box2d)

if(APPLE)
	if(NOT DEFINED OPENSSL_ROOT_DIR)
		set(OPENSSL_ROOT_DIR /opt/homebrew/Cellar/openssl@1.1/1.1.1s)
		# If this does not work, then set the path by providing:
		# -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@1.1)
	endif()
endif()

add_subdirectory(3rdparty/mosquitto)

add_subdirectory(3rdparty/zlibcomplete)

option(BOX2D_BUILD_UNIT_TESTS OFF)
option(BOX2D_BUILD_TESTBED OFF)

set (MARSIM_SOURCE_FILES
		3rdparty/stb_image.cpp
		3rdparty/implot/implot.cpp
		3rdparty/implot/implot_items.cpp
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
		src/terrain.cpp
		src/object.cpp
		src/stone.cpp
		src/proximity_sensor.cpp
		src/pickup_sensor.cpp
		src/mqtt.cpp
		src/raycast.cpp
		src/laser.cpp
		src/alien.cpp
		src/Battery.cpp
		src/friction_zone.cpp
		src/shadow_zone.cpp
		src/earthquake.cpp
		src/tornado.cpp
		src/volcano.cpp
		src/physical_weather_sensor.cpp
		src/temperature_sensor.cpp
		src/seismic_sensor.cpp
		src/wind_sensor.cpp
		src/lidar_sensor.cpp
		)

add_executable(marsim ${MARSIM_SOURCE_FILES})
target_include_directories(marsim PRIVATE src 3rdparty 3rdparty/mosquitto/include 3rdparty/zlibcomplete/zlib)
target_link_libraries(marsim PUBLIC box2d glfw imgui sajson glad libmosquitto_static zlibcomplete zlibstatic)

FILE(COPY src/data DESTINATION ${PROJECT_BINARY_DIR})

set (LISTENER_SOURCE_FILES
		src/listener.cpp
		)

add_executable(listener ${LISTENER_SOURCE_FILES})
target_include_directories(listener PRIVATE src 3rdparty 3rdparty/mosquitto/include 3rdparty/zlibcomplete/zlib)
target_link_libraries(listener PUBLIC libmosquitto_static zlibcomplete zlibstatic)
