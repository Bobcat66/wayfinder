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
#include "wfcore/common/status.h"
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <format>

namespace impl {
    static void renderText(cv::Mat& image, const std::string& text, double scale, const cv::Point& orig, const cv::Scalar& color, int thickness) {
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

    static bool validPoint(const cv::Point2d& pt) {
        return std::isfinite(pt.x) && std::isfinite(pt.y);
    }

    static constexpr double MAX_OFFSET = 1000.0; // px outside the image allowed

    static bool inReasonableBounds(const cv::Point2d& p,const cv::Mat& image) {
    return p.x >= -MAX_OFFSET && p.x <= image.cols + MAX_OFFSET &&
           p.y >= -MAX_OFFSET && p.y <= image.rows + MAX_OFFSET;
    };

}

namespace wf {
    bool drawTag3D(
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
        WPILibPose3ToCvPoseVecs(tagPose, rvecs, tvecs);
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

        for (auto& p: imgPoints) {
            if (!impl::validPoint(p) || !impl::inReasonableBounds(p,image)) return false;
        }

        // Draw base of cube
        for (int i = 0; i < 4; ++i)
            cv::line(image, imgPoints[i], imgPoints[(i + 1) % 4], Blue, 2);

        // Draw top of cube
        for (int i = 4; i < 8; ++i)
            cv::line(image, imgPoints[i], imgPoints[4 + (i + 1) % 4], Green, 2);

        // Connect vertical edges
        for (int i = 0; i < 4; ++i)
            cv::line(image, imgPoints[i], imgPoints[i + 4], Red, 2);
        
        return true;

    }
    bool drawTagAxes(
        cv::Mat& image,
        const ApriltagRelativePoseObservation& observation,
        const CameraIntrinsics& intrinsics,
        double tagSize
    ) {
        static cv::Scalar  Blue(255, 0, 0);
        static cv::Scalar Green(0, 255, 0);
        static cv::Scalar   Red(0, 0, 255);
        const auto tagPose = observation.camPose0;
        cv::Mat tvecs, rvecs;
        WPILibPose3ToCvPoseVecs(tagPose, rvecs, tvecs);
        cv::drawFrameAxes(
            image,
            intrinsics.cameraMatrix,
            intrinsics.distCoeffs,
            rvecs,
            tvecs,
            6.5
        );
        return true;
    }
    void drawBbox(cv::Mat& image, const ObjectDetection& detection) {
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
    }
    void drawTag(cv::Mat& image, const ApriltagDetection& detection) {
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
    }
    void drawCamLabel(cv::Mat& image, const std::string& camera_label) {
        static cv::Scalar White(255, 255, 255);
        impl::renderText(
            image,
            camera_label,
            0.002,
            cv::Point(3,image.rows - 3),
            White,
            2
        );
    }
}
