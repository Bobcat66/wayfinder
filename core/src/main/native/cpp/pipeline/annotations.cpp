/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "wfcore/pipeline/annotations.h"
#include "wfcore/utils/coordinates.h"
#include <opencv2/imgproc.hpp>
#include <format>

namespace impl {
    void renderText(cv::Mat& image, const std::string& text, double scale, const cv::Point& orig, const cv::Scalar& color, int thickness) {
        int imageHeight = image.rows;
        double fontScale = imageHeight * scale;
        //fontScale = std::max(fontScale, 0.5);
        cv::putText(
            image,
            text,
            orig,
            cv::FONT_HERSHEY_SIMPLEX,
            fontScale,
            color,
            thickness
        );
    }
}

namespace wf {
    int drawTag3D(
        cv::Mat& image,
        const ApriltagRelativePoseObservation& observation,
        const CameraIntrinsics& intrinsics, 
        double tagSize
    ) {
        static cv::Scalar  Blue(255, 0, 0);
        static cv::Scalar Green(0, 255, 0);
        static cv::Scalar   Red(0, 0, 255);
        const auto tagPose = observation.error0 > observation.error1
            ? observation.camPose1
            : observation.camPose0;
        cv::Mat tvecs, rvecs;
        WPILibPose3ToCvPoseVecs(tagPose, tvecs, rvecs);
        std::vector<cv::Point3d> cubePoints = {
            {0, 0, 0},
            {tagSize, 0, 0},
            {tagSize, tagSize, 0},
            {0, tagSize, 0},
            {0, 0, -tagSize},
            {tagSize, 0, -tagSize},
            {tagSize, tagSize, -tagSize},
            {0, tagSize, -tagSize}
        };

        // Project 3D points to 2D image points
        std::vector<cv::Point2d> imgPoints;
        cv::projectPoints(
            cubePoints, 
            rvecs, tvecs, 
            intrinsics.cameraMatrix, 
            intrinsics.distCoeffs, 
            imgPoints
        );

        // Draw base of cube
        for (int i = 0; i < 4; ++i)
            cv::line(image, imgPoints[i], imgPoints[(i + 1) % 4], Blue, 2);

        // Draw top of cube
        for (int i = 4; i < 8; ++i)
            cv::line(image, imgPoints[i], imgPoints[4 + (i + 1) % 4], Green, 2);

        // Connect vertical edges
        for (int i = 0; i < 4; ++i)
            cv::line(image, imgPoints[i], imgPoints[i + 4], Red, 2);
        
        return 0;

    }
    int drawBbox(cv::Mat& image, const ObjectDetection& detection) {
        static cv::Scalar Red(0,0,255);
        cv::rectangle(image,
            detection.bboxTopLeftPixels,
            detection.bboxBottomRightPixels,
            Red
        );
        cv::Point labelOrig(
            static_cast<int>(detection.bboxTopLeftPixels.x),
            static_cast<int>(detection.bboxTopLeftPixels.y - 2)
        );
        impl::renderText(
            image,
            std::format("Class {}: {:.3f}", detection.objectClass, detection.confidence),
            0.001,
            labelOrig,
            Red,
            1
        );

        return 0;
    }
    int drawTag(cv::Mat& image, const ApriltagDetection& detection) {
        static cv::Scalar Green(0,255,0);
        for (int i = 0; i < 4; ++i) {
            cv::line(
                image,
                detection.corners[i % 4],
                detection.corners[(i + 1) % 4],
                Green,
                2
            );
        }
        cv::Point labelOrig(
            static_cast<int>(detection.corners[0].x),
            static_cast<int>(detection.corners[0].y - 2)
        );
        impl::renderText(
            image,
            std::format("{}", detection.id),
            0.001,
            labelOrig,
            Green,
            1
        );
        return 0;
    }
    int drawCamLabel(cv::Mat& image, const std::string& camera_label) {
        static cv::Scalar White(255, 255, 255);
        impl::renderText(
            image,
            camera_label,
            0.002,
            cv::Point(3,image.rows - 3),
            White,
            2
        );
        return 0;
    }
}
