cmake_minimum_required(VERSION 3.0)

project(ZLIBCOMPLETE)

# Generate compilation database for YouCompleteMe and stuff like clang-tidy.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Allow some tweaks in build process without changing CMakeLists.
option(ZLIBCOMPLETE_WERROR  "Build with -Werror flag."                 OFF)
option(ZLIBCOMPLETE_WSTRICT "Build with really strict warning flags."  OFF)
option(ZLIBCOMPLETE_RELEASE "Build with optimizations"                 ON)

# Check if used compiler supports warning flags we use.
if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
   ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR
   ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
    if (ZLIBCOMPLETE_WSTRICT)
        set(WARNING_FLAGS ${WARNING_FLAGS} -Wall -Wextra -pedantic -Weffc++
                -Wcast-align -Wcast-qual -Wno-ctor-dtor-privacy
                -Wdisabled-optimization -Wformat=2 -Winit-self
                -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast
                -Woverloaded-virtual -Wredundant-decls -Wshadow
                -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch
                -Wundef -Wno-unused -Wnon-virtual-dtor -Wreorder -Wdeprecated
                -Wno-float-equal)
    endif()
    if(ZLIBCOMPLETE_WERROR)
        set(WARNING_FLAGS ${WARNING_FLAGS} -Werror)
    endif()
else()
    message(WARNING "Using WERROR/WSTRICT with non-GCC like compiler is not supported. Ignoring.")
endif()

# Library itself

#find_package(ZLIB REQUIRED)
add_subdirectory(zlib) # <-- Added as subdirectory here!


option(ZLIBCOMPLETE_STATIC "Build static library." ON )
option(ZLIBCOMPLETE_SHARED "Build shared library." OFF)

file(GLOB_RECURSE ZLIBCOMPLETE_HEADERS ${PROJECT_SOURCE_DIR}/lib/*.h)
file(GLOB_RECURSE ZLIBCOMPLETE_SOURCES ${PROJECT_SOURCE_DIR}/lib/*.cpp)

if(ZLIBCOMPLETE_STATIC)
    add_library(zlibcomplete STATIC ${ZLIBCOMPLETE_HEADERS} ${ZLIBCOMPLETE_SOURCES})
endif()

if(ZLIBCOMPLETE_SHARED)
    add_library(zlibcomplete SHARED ${ZLIBCOMPLETE_HEADERS} ${ZLIBCOMPLETE_SOURCES})
endif()

target_link_libraries(zlibcomplete PUBLIC ${ZLIB_LIBRARIES})
target_include_directories(zlibcomplete PUBLIC ${PROJECT_SOURCE_DIR}/lib zlib ${CMAKE_BINARY_DIR}/3rdparty/zlibcomplete/zlib)
target_compile_options(zlibcomplete PRIVATE ${WARNING_FLAGS})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/zlibcomplete
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
       )
install(TARGETS zlibcomplete
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
       )
