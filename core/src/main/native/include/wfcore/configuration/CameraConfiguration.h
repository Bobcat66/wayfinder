#pragma once

#include <opencv2/core.hpp>

namespace wf {
    
    struct CameraIntrinsics {
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    };

    struct CameraConfiguration {
        int id;
        CameraIntrinsics intrinsics; 
    };

}