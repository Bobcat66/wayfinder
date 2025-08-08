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

#include "wfcore/fiducial/ApriltagDetector.h"
#include "wfcore/pipeline/annotations.h"
#include <opencv2/opencv.hpp>
#include "wfcore/video/processing.h"
#include <iostream>
#include "wfcore/pipeline/pnp.h"

static cv::Mat createIntrinsicsMatrix(double fx, double fy, double cx, double cy) {
    return (cv::Mat_<double>(3, 3) <<
        fx,  0, cx,
         0, fy, cy,
         0,  0,  1);
}


int main() {
    auto distortionMat = cv::Mat(std::vector<double>{
        0.09581952042360092,
        -0.2603932345361037,
        0.0035795949814343524,
        -0.005134231272255606,
        0.19101200082384226
    }).reshape(1,1).clone();
    auto matrixMat = createIntrinsicsMatrix(
        979.1087360312252,
        979.8457780935689,
        608.5591334099096,
        352.9815581130428
    );
    wf::CameraIntrinsics intrinsics(
        {1280,720},
        matrixMat,
        distortionMat
    );
    auto capture = cv::VideoCapture(0);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cv::Mat frame;
    cv::Mat grayFrame;
    std::vector<std::unique_ptr<wf::CVProcessNode<cv::Mat>>> nodes;
    nodes.emplace_back(std::make_unique<wf::ColorConvertNode<cv::Mat>>(wf::ImageEncoding::Y8));
    wf::CVProcessPipe<cv::Mat> preprocessor(
        wf::FrameFormat(wf::ImageEncoding::BGR24,720,1280),
        std::move(nodes)
    );
    wf::ApriltagDetector detector;
    std::cout << "Initialized Demo" << std::endl;
    detector.addFamily("tag36h11");
    wf::ApriltagConfiguration apriltagConfiguration("tag36h11", 6.5);
    while (true) {
        bool success = capture.read(frame);
        if (!success || frame.empty()) {
            std::cerr << "Error: Can't read frame\n";
            break;
        }
        preprocessor.processDirect(frame,grayFrame);
        auto detres = detector.detect(grayFrame);
        if (!detres) break;
        auto detections = std::move(detres.value());
        std::vector<wf::ApriltagRelativePoseObservation> observations;
        for (auto detection : detections) {
            auto pnp_obs_opt = wf::solvePNPApriltagRelative(
                detection,
                apriltagConfiguration,
                intrinsics
            );
            if (pnp_obs_opt) observations.push_back(pnp_obs_opt.value());
        }
        for (auto detection : detections) {
            wf::drawTag(frame,detection);
        }
        for (auto observation: observations) {
            if(!wf::drawTag3D(frame,observation,intrinsics,6.5))
                std::cerr << "Failed to render 3D box\n";
        }
        wf::drawCamLabel(frame,"/dev/video0");
        cv::imshow("OpenCV Apriltag Demo",frame);
        if (cv::waitKey(30) == 27) break;
    }
}