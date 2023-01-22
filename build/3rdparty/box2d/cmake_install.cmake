# Install script for directory: C:/Users/ermia/OneDrive/Desktop/IPW/marsim/3rdparty/box2d

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/marsim")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/ProgramData/chocolatey/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "C:/Users/ermia/OneDrive/Desktop/IPW/marsim/3rdparty/box2d/include/box2d")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/ermia/OneDrive/Desktop/IPW/marsim/build/3rdparty/box2d/src/cmake_install.cmake")
  include("C:/Users/ermia/OneDrive/Desktop/IPW/marsim/build/3rdparty/box2d/unit-test/cmake_install.cmake")
  include("C:/Users/ermia/OneDrive/Desktop/IPW/marsim/build/3rdparty/box2d/extern/glad/cmake_install.cmake")
  include("C:/Users/ermia/OneDrive/Desktop/IPW/marsim/build/3rdparty/box2d/extern/glfw/cmake_install.cmake")
  include("C:/Users/ermia/OneDrive/Desktop/IPW/marsim/build/3rdparty/box2d/extern/imgui/cmake_install.cmake")
  include("C:/Users/ermia/OneDrive/Desktop/IPW/marsim/build/3rdparty/box2d/extern/sajson/cmake_install.cmake")
  include("C:/Users/ermia/OneDrive/Desktop/IPW/marsim/build/3rdparty/box2d/testbed/cmake_install.cmake")

endif()

