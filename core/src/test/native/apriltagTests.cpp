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
#include "wfcore/common/logging/LoggerManager.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <filesystem>
#include <gtest/gtest.h>

// This is for development environments only
#define RESOURCE_PATH "../../test-resources"

#define STRINGIZE_IMPL(x) #x
#define TO_STRING(x) STRINGIZE_IMPL(x)

#define DETECTION_TEST(testname,imgname,tagfamily,detconfig,qtps)                                               \
    TEST(apriltagTests,testname) {                                                                              \
        cv::Mat image = cv::imread(RESOURCE_PATH "/apriltag/" imgname);                                         \
        logger->info("Accessed {}", RESOURCE_PATH "/apriltag/" imgname);                                        \
        cv::Mat gray;                                                                                           \
        cv::cvtColor(image,gray,cv::COLOR_BGR2GRAY);                                                            \
        ASSERT_FALSE(gray.empty()) << "Gray image is empty before detection";                                   \
        ASSERT_EQ(gray.type(), CV_8UC1) << "Gray image is not single-channel 8-bit";                            \
        wf::ApriltagDetectorConfig testConfig = detconfig;                                                      \
        wf::QuadThresholdParams testQTPs = qtps;                                                                \
        wf::ApriltagDetector detector;                                                                          \
        detector.setConfig(testConfig);                                                                         \
        logger->info("Detector config set to:\n{}",detector.getConfig().string());                              \
        detector.setQuadThresholdParams(testQTPs);                                                              \
        logger->info("Detector QTPs set to:\n{}",detector.getQuadThresholdParams().string());                   \
        logger->info("Adding tag family {}", tagfamily);                                                        \
        int err = detector.addFamily(tagfamily);                                                                \
        logger->info("Add family op returned {}",err);                                                          \
        auto res = detector.detect(gray);                                                                       \
        logger->info("Detected {} apriltags",res.size());                                                       \
        for (auto detection : res) {                                                                            \
            auto top_left = detection.corners[0];                                                               \
            auto top_right = detection.corners[1];                                                              \
            auto bottom_right = detection.corners[2];                                                           \
            auto bottom_left = detection.corners[3];                                                            \
            cv::Scalar green(0,255,0);                                                                          \
            cv::line(image,top_left,top_right,green);                                                           \
            cv::line(image,top_right,bottom_right,green);                                                       \
            cv::line(image,bottom_right,bottom_left,green);                                                     \
            cv::line(image,bottom_left,top_left,green);                                                         \
        }                                                                                                       \
        std::filesystem::path tempDir                                                                           \
            = std::filesystem::temp_directory_path()                                                            \
            / TO_STRING(testname) "_output.jpg";                                                                \
        logger->info("{} output written to: {}",TO_STRING(testname), tempDir.string());                         \
        bool out = cv::imwrite(tempDir,image);                                                                  \
    }


static wf::loggerPtr logger = wf::LoggerManager::getInstance().getLogger("apriltagTests",wf::LogGroup::General);

TEST(apriltagTests,detectorConfigurationTest) {
    wf::ApriltagDetector detector;
    auto defaultConfig = detector.getConfig();
    auto defaultQTPs = detector.getQuadThresholdParams();
    wf::ApriltagDetectorConfig newConfig{
        .numThreads = 4
    };
    wf::QuadThresholdParams newQTPs{
        .minClusterPixels = 5,
        .deglitch = true
    };
    detector.setConfig(newConfig);
    detector.setQuadThresholdParams(newQTPs);
    EXPECT_EQ(wf::ApriltagDetectorConfig(),defaultConfig);
    EXPECT_EQ(wf::QuadThresholdParams(),defaultQTPs);
    EXPECT_EQ(newConfig,detector.getConfig());
    EXPECT_EQ(newQTPs,detector.getQuadThresholdParams());
    EXPECT_NE(defaultConfig,detector.getConfig());
    EXPECT_NE(defaultQTPs,detector.getQuadThresholdParams());
}


DETECTION_TEST(
    cubesDetectionTest,
    "cubes.jpg",
    "tag36h11",
    {
        .debug=true
    },
    {}
)

DETECTION_TEST(
    robotsDetectionTest,
    "robots.jpg",
    "tag36h11",
    {
        .debug=true
    },
    {
        .minClusterPixels = 10
    }
)