# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseExamplesAlign)

if(BUILD_GRAPHICAL_EXAMPLES)
    add_executable(rs-align-test rs-align-test.cpp ../../third-party/imgui/imgui.cpp ../../third-party/imgui/imgui_draw.cpp ../../third-party/imgui/imgui_impl_glfw.cpp)
    set_property(TARGET rs-align PROPERTY CXX_STANDARD 11)
    target_link_libraries(rs-align-test ${DEPENDENCIES})
    include_directories(rs-align-test ../../common ../../third-party/imgui ../../third-party ../../third-party/tclap/include)
    set_target_properties (rs-align-test PROPERTIES FOLDER Examples)
    install(TARGETS rs-align-test RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
