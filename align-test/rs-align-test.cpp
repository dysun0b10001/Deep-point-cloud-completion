// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2019 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp>
#include "../example.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"

#include <fstream>  // File IO
#include <iostream> // Terminal IO
#include <sstream>  // Stringstreams
#include <iomanip>
#include <limits>
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <array>
#include <string>
#include <vector>
#ifdef WIN32
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem::v1;
#else
#if __has_include(<filesystem>)
#include <filesystem>
namespace filesystem = std::filesystem;
#else
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#endif
#endif

// 3rd party header for writing png files
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// initialization
void initialize(int argc, char * argv[], filesystem::path & home_dir, uint32_t & frame_int)
{
    const std::string keys =
        "{ home h    |       | home saving directory, required (path)                            }"
        "{ interval i| 1     | frame interval of saved images, optional, default is 1 (uint32_t) }";
    cv::CommandLineParser parser( argc, argv, keys );

    // Check Parsing Error
    if( !parser.check() ){
        parser.printErrors();
        throw std::runtime_error( "failed command arguments" );
    }

    // get home directory and create subdirectory for color and depth images
    if( !parser.has("home")){
        throw std::runtime_error( "no home saving directory provided");
    }
    else{
        home_dir = parser.get<std::string>("home");
        filesystem::path color_dir = home_dir.generic_string() + "/Color";
        filesystem::path depth_dir = home_dir.generic_string() + "/Depth";
        filesystem::create_directories(color_dir);
        filesystem::create_directories(depth_dir);
    }

    // get frame interval (Option)
    if( !parser.has( "interval" ) ){
        frame_int = 1;
    }
    else{
        frame_int = parser.get<uint32_t>( "interval" );
    } 
}


// Draw Depth
cv::Mat drawDepth(rs2::depth_frame depth)
{
    cv::Mat depth_mat;
    // Create cv::Mat form Depth Frame
    depth_mat = cv::Mat( depth.get_height(), depth.get_width(), CV_16UC1, const_cast<void*>( depth.get_data() ) ).clone();
    return depth_mat;
}

// Save Depth
void saveDepth(cv::Mat depth_mat, rs2::depth_frame depth, filesystem::path home_dir, uint32_t frame_num, uint32_t frame_int)
{
    if( !depth ){
        return;
    }

    if( depth_mat.empty() ){
        return;
    }

    // Create Save Directory and File Name
    std::ostringstream oss;
    oss << home_dir.generic_string() << "/Depth/";
    oss << "depth_" << frame_num/frame_int + 1 << ".png";
    std::cout << "depth_" << frame_num/frame_int + 1 << ".png" << " saved."<<std::endl;

    // Scalinge
    cv::Mat scale_mat = depth_mat;
    // if( scaling ){
    //     depth_mat.convertTo( scale_mat, CV_8U, -255.0 / 10000.0, 255.0 ); // 0-10000 -> 255(white)-0(black)
    // }

    // Write Depth Image
    cv::imwrite( oss.str(), scale_mat );
}
/*
 This example introduces the concept of spatial stream alignment.
 For example usecase of alignment, please check out align-advanced and measure demos.
 The need for spatial alignment (from here "align") arises from the fact
 that not all camera streams are captured from a single viewport.
 Align process lets the user translate images from one viewport to another. 
 That said, results of align are synthetic streams, and suffer from several artifacts:
 1. Sampling - mapping stream to a different viewport will modify the resolution of the frame 
               to match the resolution of target viewport. This will either cause downsampling or
               upsampling via interpolation. The interpolation used needs to be of type
               Nearest Neighbor to avoid introducing non-existing values.
 2. Occlussion - Some pixels in the resulting image correspond to 3D coordinates that the original
               sensor did not see, because these 3D points were occluded in the original viewport.
               Such pixels may hold invalid texture values.
*/

// This example assumes camera with depth and color
// streams, and direction lets you define the target stream
enum class direction
{
    to_depth,
    to_color
};

// Forward definition of UI rendering, implemented below
void render_slider(rect location, float *alpha, direction *dir);

int main(int argc, char *argv[]) try
{
    cv::Mat depth_mat;
    uint32_t frame_num = 0;
    uint32_t frame_int;
    filesystem::path home_dir;
    std::vector <std::string> data_list;

    initialize(argc, argv, home_dir, frame_int);

    // Create and initialize GUI related objects
    window app(1280, 720, "RealSense Align Example"); // Simple window handling
    ImGui_ImplGlfw_Init(app, false);                  // ImGui library intializition
    rs2::colorizer c;                                 // Helper to colorize depth images
    texture depth_image, color_image;                 // Helpers for renderig images

    // Create a pipeline to easily configure and start the camera
    rs2::pipeline pipe;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH);
    cfg.enable_stream(RS2_STREAM_COLOR);
    pipe.start(cfg);

    // Define two align objects. One will be used to align
    // to depth viewport and the other to color.
    // Creating align object is an expensive operation
    // that should not be performed in the main loop
    rs2::align align_to_depth(RS2_STREAM_DEPTH);
    rs2::align align_to_color(RS2_STREAM_COLOR);

    float alpha = 0.5f;                  // Transparancy coefficient
    direction dir = direction::to_color; // Alignment direction

    while (app) // Application still alive?
    {
        // Using the align object, we block the application until a frameset is available
        rs2::frameset frameset = pipe.wait_for_frames();

        if (dir == direction::to_depth)
        {
            // Align all frames to depth viewport
            frameset = align_to_depth.process(frameset);
        }
        else
        {
            // Align all frames to color viewport
            frameset = align_to_color.process(frameset);
        }

        // With the aligned frameset we proceed as usual
        auto depth = frameset.get_depth_frame();
        auto color = frameset.get_color_frame();
        auto colorized_depth = c.colorize(depth);

        // // Write images to disk
        // std::stringstream png_file1;
        // png_file1 << "rs-save-to-disk-depth" << ".png";
        // stbi_write_png(png_file1.str().c_str(), depth.get_width(), depth.get_height(),
        //                depth.get_bytes_per_pixel(), depth.get_data(), depth.get_stride_in_bytes());
        // std::cout << "Saved " << png_file1.str() << std::endl;
        // std::cout << "Data depth " << depth.get_bytes_per_pixel() << std::endl;

        if(frame_num % frame_int == 0){
            // Draw Depth
            depth_mat = drawDepth(depth);
            // Save Depth
            saveDepth(depth_mat,depth,home_dir,frame_num,frame_int);

            std::stringstream png_file2;
            // Write color images to disk
            png_file2 << home_dir.generic_string() << "/Color/color_" << frame_num/frame_int + 1 << ".png";
            stbi_write_png(png_file2.str().c_str(), color.get_width(), color.get_height(),
                        color.get_bytes_per_pixel(), color.get_data(), color.get_stride_in_bytes());
            std::cout << "color_" << frame_num/frame_int + 1 << ".png" << " saved."<<std::endl;
            //std::cout << "Data color " << color.get_bytes_per_pixel() << std::endl;

            // add to data_list file
            data_list.push_back(std::to_string(frame_num/frame_int + 1));
            data_list.push_back("\n");
        }

        glEnable(GL_BLEND);
        // Use the Alpha channel for blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (dir == direction::to_depth)
        {
            // When aligning to depth, first render depth image
            // and then overlay color on top with transparancy
            depth_image.render(colorized_depth, {0, 0, app.width(), app.height()});
            color_image.render(color, {0, 0, app.width(), app.height()}, alpha);
        }
        else
        {
            // When aligning to color, first render color image
            // and then overlay depth image on top
            color_image.render(color, {0, 0, app.width(), app.height()});
            depth_image.render(colorized_depth, {0, 0, app.width(), app.height()}, 1 - alpha);
        }

        glColor4f(1.f, 1.f, 1.f, 1.f);
        glDisable(GL_BLEND);

        // Render the UI:
        ImGui_ImplGlfw_NewFrame(1);
        render_slider({15.f, app.height() - 60, app.width() - 30, app.height()}, &alpha, &dir);
        ImGui::Render();

        frame_num += 1;
    }

    std::ofstream file;
    file.open(home_dir.generic_string()+"/data_list.txt");
    for (std::vector<std::string>::iterator iter = data_list.begin(); iter < data_list.end()-1; ++iter){
        file << *iter;
    }
    file.close();
    std::cout << std::endl;
    std::cout << "finish, "<< frame_num/frame_int + 1<< " frame(s) saved."<<std::endl;

    return EXIT_SUCCESS;
}
catch (const rs2::error &e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception &e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

void render_slider(rect location, float *alpha, direction *dir)
{
    static const int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos({location.x, location.y});
    ImGui::SetNextWindowSize({location.w, location.h});

    // Render transparency slider:
    ImGui::Begin("slider", nullptr, flags);
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("##Slider", alpha, 0.f, 1.f);
    ImGui::PopItemWidth();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Texture Transparancy: %.3f", *alpha);

    // Render direction checkboxes:
    bool to_depth = (*dir == direction::to_depth);
    bool to_color = (*dir == direction::to_color);

    if (ImGui::Checkbox("Align To Depth", &to_depth))
    {
        *dir = to_depth ? direction::to_depth : direction::to_color;
    }
    ImGui::SameLine();
    ImGui::SetCursorPosX(location.w - 140);
    if (ImGui::Checkbox("Align To Color", &to_color))
    {
        *dir = to_color ? direction::to_color : direction::to_depth;
    }

    ImGui::End();
}


