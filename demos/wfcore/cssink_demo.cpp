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

#include "wfcore/video/CSCameraSink.h"
#include "wfcore/video/video_utils.h"
#include <opencv2/opencv.hpp>
#include <format>

int main() {
    cv::Mat frame;
    cs::UsbCamera cs_source("test_camera","/dev/video0");
    cs::CvSink cs_sink("test_sink",cs::VideoMode::PixelFormat::kBGR);
    cs_sink.SetSource(cs_source);
    wf::CSCameraSink wfsink(
        "test_sink","/dev/video0",std::move(cs_sink),
        wf::getStreamFormatFromVideoMode(cs_source.GetVideoMode())
    );
    std::cout << "Entering loop" << std::endl;
    while (true) {
        std::cout << "Getting frame" << std::endl;
        auto meta = wfsink.getFrame(frame);
        std::cout << "OpenCV Mat type: " << frame.type() << std::endl;
        std::cout << "Channels: " << frame.channels() << std::endl;
        std::cout << "Depth: " << frame.depth() << std::endl;
        if (meta.err()) {
            std::cout << "Error getting frame: " << wfsink.getError() << std::endl;
        } else {
            cv::imshow("Wayfinder CS Sink Demo",frame);
        }
        if (cv::waitKey(30) == 27) break;
    }

}