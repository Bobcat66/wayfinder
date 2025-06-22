#pragma once

#include <iostream>
#include <filesystem>
#include <string>
namespace wf {
    enum class ApriltagTestImage {
        CUBES,
        ROBOTS
    };
    std::filesystem::path getResourcePath();

    std::filesystem::path getApriltagTestImagePath(ApriltagTestImage image);
}