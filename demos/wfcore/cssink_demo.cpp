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

#include "wfcore/hardware/CSCameraSink.h"
#include "wfcore/hardware/CSCameraHandler.h"
#include "wfcore/hardware/HardwareManager.h"
#include "wfcore/video/video_utils.h"
#include <opencv2/opencv.hpp>
#include <format>
#include <cstdlib>

int main() {
    cv::Mat frame;
    cs::UsbCamera cs_source("test_camera","/dev/video0");
    wf::HardwareManager manager;
    manager.registerCamera(
        {
            "test_camera",
            "/dev/video0",
            wf::CameraBackend::CSCORE,
            {
                30,
                {
                    wf::ImageEncoding::YUYV,
                    640,
                    480
                }
            },
            {},
            {},
            {}
        }
    );
    auto sinkres = manager.getFrameProvider("test_camera","test_sink");
    if (!sinkres)
        throw wf::wf_result_error(sinkres);
    auto sink = std::move(sinkres.value());
    std::cout << "Entering loop" << std::endl;
    while (true) {
        std::cout << "Getting frame" << std::endl;
        auto meta = sink->getFrame(frame);
        std::cout << "OpenCV Mat type: " << frame.type() << std::endl;
        std::cout << "Channels: " << frame.channels() << std::endl;
        std::cout << "Depth: " << frame.depth() << std::endl;
        if (meta.err()) {
            // TODO: Make this not terrible
            std::cout << "Error getting frame" << std::endl;
        } else {
            cv::imshow("Wayfinder CS Sink Demo",frame);
        }
        if (cv::waitKey(30) == 27) break;
    }

}