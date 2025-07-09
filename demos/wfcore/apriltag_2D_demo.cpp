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

int main() {
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
    while (true) {
        bool success = capture.read(frame);
        if (!success || frame.empty()) {
            std::cerr << "Error: Can't read frame\n";
            break;
        }
        preprocessor.processDirect(frame,grayFrame);
        auto detections = detector.detect(grayFrame);
        for (auto detection : detections) {
            wf::drawTag(frame,detection);
        }
        wf::drawCamLabel(frame,"/dev/video0");
        cv::imshow("OpenCV Apriltag Demo",frame);
        if (cv::waitKey(30) == 27) break;
    }
}