# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseExamplesAlign)

# Find Package
# librealsense2
set( realsense2_DIR "C:/Program Files/librealsense2/lib/cmake/realsense2" CACHE PATH "Path to librealsense2 config directory." )
find_package( realsense2 REQUIRED )

# OpenCV
set( OpenCV_DIR "C:/Program Files/opencv/build" CACHE PATH "Path to OpenCV config directory." )
find_package( OpenCV REQUIRED )

if(BUILD_GRAPHICAL_EXAMPLES)
    add_executable(rs-align-test rs-align-test.cpp ../../third-party/imgui/imgui.cpp ../../third-party/imgui/imgui_draw.cpp ../../third-party/imgui/imgui_impl_glfw.cpp)
    set_property(TARGET rs-align PROPERTY CXX_STANDARD 11)
    target_link_libraries(rs-align-test ${DEPENDENCIES})
    target_link_libraries( rs-align-test ${OpenCV_LIBS} )
    include_directories(rs-align-test ../../common ../../third-party/imgui ../../third-party ../../third-party/tclap/include)
    set_target_properties (rs-align-test PROPERTIES FOLDER Examples)
    install(TARGETS rs-align-test RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
