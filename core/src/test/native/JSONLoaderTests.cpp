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

#include "wfcore/configuration/ResourceManager.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include <gtest/gtest.h>
#include <filesystem>
#include "wfcore/common/logging.h"
#include <algorithm>
#include <iostream>

// This is for development environments only. Tests will NOT work once installed
#define MOCK_LOCAL_PATH "../../../test-resources/mock_local"
#define MOCK_RESOURCE_PATH "../../../resources"

namespace fs = std::filesystem;

TEST(JSONLoaderTests,localLoadTest) {
    wf::globalLogger()->info("Entering localLoadTest");
    wf::globalLogger()->info("Mock local path: {}",MOCK_LOCAL_PATH);
    wf::globalLogger()->info("Mock resource path: {}",MOCK_RESOURCE_PATH);
    fs::path mock_lpath(MOCK_LOCAL_PATH);
    fs::path mock_rpath(MOCK_RESOURCE_PATH);
    wf::globalLogger()->info("Created path objects");
    wf::ResourceManager loader;
    try {
        loader = wf::ResourceManager(mock_rpath,mock_lpath);
    } catch (const wf::wfexception& e) {
        wf::globalLogger()->info("Caught wfexception");
        FAIL() << e.what();
    }

    wf::globalLogger()->info("Created JSONLoader");
    ASSERT_TRUE(loader.assignLocalSubdir("hardware",fs::path("hardware")));
    wf::globalLogger()->info("Assigned local subdir");

    std::vector<std::string> localSubdirs = loader.enumerateLocalSubdirs();
    EXPECT_TRUE(std::find(localSubdirs.begin(),localSubdirs.end(),"hardware") != localSubdirs.end());

    auto enumRes = loader.enumerateLocalSubdir("hardware");
    ASSERT_TRUE(enumRes);

    auto hardwareConfigs = std::move(enumRes.value());
    EXPECT_TRUE(std::find(hardwareConfigs.begin(),hardwareConfigs.end(),"mock_camera.json") != hardwareConfigs.end());

    auto jsonRes = loader.loadLocalJSON("hardware","mock_camera.json");
    ASSERT_TRUE(jsonRes);
    wf::globalLogger()->info("Loaded JSON file");

    auto mockconf_jobject = std::move(jsonRes.value());
    std::cout << mockconf_jobject.dump(4) << std::endl;

    auto mkcfRes = wf::CameraConfiguration::fromJSON(mockconf_jobject);
    ASSERT_TRUE(mkcfRes);
    wf::globalLogger()->info("Parsed JSON file");

    wf::globalLogger()->info("Validating parsed camera configuration");
    auto mockconf = std::move(mkcfRes.value());
    EXPECT_EQ("mock_camera",mockconf.nickname);
    EXPECT_EQ("/dev/video0",mockconf.devpath);
    EXPECT_EQ(wf::CameraBackend::CSCORE,mockconf.backend);
    EXPECT_EQ(wf::StreamFormat(10,{wf::ImageEncoding::YUYV,1280,720}),mockconf.format);
    std::unordered_map<wf::CamControl,std::string> expectedAliases = {
        {wf::CamControl::BRIGHTNESS,"brightness"},
        {wf::CamControl::CONTRAST,"contrast"},
        {wf::CamControl::SATURATION,"saturation"},
        {wf::CamControl::HUE,"hue"},
        {wf::CamControl::AUTO_WHITE_BALANCE,"white_balance_automatic"},
        {wf::CamControl::SHARPNESS,"sharpness"},
        {wf::CamControl::AUTO_EXPOSURE,"auto_exposure"}
    };
    for (const auto& [key,value] : expectedAliases) {
        auto it = mockconf.controlAliases.find(key);
        EXPECT_FALSE(it == mockconf.controlAliases.end());
        if (it != mockconf.controlAliases.end()) {
            EXPECT_EQ(value,it->second);
        }
    }
    EXPECT_TRUE(mockconf.calibrations.size() == 1);
    if (mockconf.calibrations.size() == 1) {
        auto calib = mockconf.calibrations[0];

        // Matrix verification
        // fx
        EXPECT_NEAR(979.1087360312252,calib.cameraMatrix.at<double>(0,0),0.000001); // A tolerance is set because of floating point rounding errors
        // fy
        EXPECT_NEAR(979.8457780935689,calib.cameraMatrix.at<double>(1,1),0.000001);
        // cx
        EXPECT_NEAR(608.5591334099096,calib.cameraMatrix.at<double>(0,2),0.000001);
        // cy
        EXPECT_NEAR(352.9815581130428,calib.cameraMatrix.at<double>(1,2),0.000001);

        std::vector<double> expectedDistvec = {
            0.09581952042360092,
            -0.2603932345361037,
            0.0035795949814343524,
            -0.005134231272255606,
            0.19101200082384226
        };

        std::vector<double> distvec(calib.distCoeffs.begin<double>(), calib.distCoeffs.end<double>());
        // Distortion verification
        EXPECT_EQ(5,distvec.size());
        for (int i = 0; i < 5; ++i) {
            try {
                EXPECT_NEAR(expectedDistvec[i],distvec[i],0.000001);
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                break;
            }
        }
    }

    std::unordered_map<wf::CamControl,int> expectedControls = {
        {wf::CamControl::BRIGHTNESS,128},
        {wf::CamControl::CONTRAST,32},
        {wf::CamControl::AUTO_WHITE_BALANCE,1}
    };
    for (const auto& [key,value] : expectedControls) {
        auto it = mockconf.controls.find(key);
        EXPECT_FALSE(it == mockconf.controls.end());
        if (it != mockconf.controls.end()) {
            EXPECT_EQ(value,it->second);
        }
    }


}