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

#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/configuration/ResourceManager.h"
#include "wfcore/video/video_types.h"
#include <gtest/gtest.h>
#include <iostream>
#include "wfcore/common/testutils.h"

TEST(JSONTests,videoTypesTest) {
    wf::StreamFormat test_format_0_orig(30,{wf::ImageEncoding::BGR24,720,1280});
    wf::StreamFormat test_format_1_orig(15,{wf::ImageEncoding::MJPEG,480,640});
    wf::StreamFormat test_format_2_orig(100,{wf::ImageEncoding::YUYV,200,200});
    wf::StreamFormat test_format_3_orig(10,{wf::ImageEncoding::RGB24,100,100});
    wf::StreamFormat test_format_4_orig(20,{wf::ImageEncoding::Y8,340,340});

    wf::JSON test_format_0_jobject;
    wf::JSON test_format_1_jobject;
    wf::JSON test_format_2_jobject;
    wf::JSON test_format_3_jobject;
    wf::JSON test_format_4_jobject;

    auto jres0 = wf::StreamFormat::toJSON(test_format_0_orig);
    ASSERT_TRUE(jres0);
    test_format_0_jobject = std::move(jres0.value());
    std::cout << "Test Format 0 JSON: " << test_format_0_jobject.dump() << std::endl;

    auto jres1 = wf::StreamFormat::toJSON(test_format_1_orig);
    ASSERT_TRUE(jres1);
    test_format_1_jobject = std::move(jres1.value());
    std::cout << "Test Format 1 JSON: " << test_format_1_jobject.dump() << std::endl;

    auto jres2 = wf::StreamFormat::toJSON(test_format_2_orig);
    ASSERT_TRUE(jres2);
    test_format_2_jobject = std::move(jres2.value());
    std::cout << "Test Format 2 JSON: " << test_format_2_jobject.dump() << std::endl;

    auto jres3 = wf::StreamFormat::toJSON(test_format_3_orig);
    ASSERT_TRUE(jres3);
    test_format_3_jobject = std::move(jres3.value());
    std::cout << "Test Format 3 JSON: " << test_format_3_jobject.dump() << std::endl;

    auto jres4 = wf::StreamFormat::toJSON(test_format_4_orig);
    ASSERT_TRUE(jres4);
    test_format_4_jobject = std::move(jres4.value());
    std::cout << "Test Format 4 JSON: " << test_format_4_jobject.dump() << std::endl;

    wf::StreamFormat test_format_0_decode;
    wf::StreamFormat test_format_1_decode;
    wf::StreamFormat test_format_2_decode;
    wf::StreamFormat test_format_3_decode;
    wf::StreamFormat test_format_4_decode;

    auto dres0 = wf::StreamFormat::fromJSON(test_format_0_jobject);
    ASSERT_TRUE(dres0);
    test_format_0_decode = std::move(dres0.value());
    std::cout << "Decoded test format 0: " << test_format_0_decode.dump() << std::endl;
    EXPECT_EQ(test_format_0_orig,test_format_0_decode);

    auto dres1 = wf::StreamFormat::fromJSON(test_format_1_jobject);
    ASSERT_TRUE(dres1);
    test_format_1_decode = std::move(dres1.value());
    std::cout << "Decoded test format 1: " << test_format_1_decode.dump() << std::endl;
    EXPECT_EQ(test_format_1_orig,test_format_1_decode);

    auto dres2 = wf::StreamFormat::fromJSON(test_format_2_jobject);
    ASSERT_TRUE(dres2);
    test_format_2_decode = std::move(dres2.value());
    std::cout << "Decoded test format 2: " << test_format_2_decode.dump() << std::endl;
    EXPECT_EQ(test_format_2_orig,test_format_2_decode);

    auto dres3 = wf::StreamFormat::fromJSON(test_format_3_jobject);
    ASSERT_TRUE(dres3);
    test_format_3_decode = std::move(dres3.value());
    std::cout << "Decoded test format 3: " << test_format_3_decode.dump() << std::endl;
    EXPECT_EQ(test_format_3_orig,test_format_3_decode);

    auto dres4 = wf::StreamFormat::fromJSON(test_format_4_jobject);
    ASSERT_TRUE(dres4);
    test_format_4_decode = std::move(dres4.value());
    std::cout << "Decoded test format 4: " << test_format_4_decode.dump() << std::endl;
    EXPECT_EQ(test_format_4_orig,test_format_4_decode);
}

TEST(JSONTests,hardwareTypesTest) {
    wf::CameraIntrinsics test_intrinsics_0_orig(
        {1280,720},
        wf::test::mockMatrix(10.0,10.0,20.0,20.0),
        wf::test::mockDistortion({1.0,2.0,3.0,4.0,5.0})
    );
    wf::CameraIntrinsics test_intrinsics_1_orig(
        {420,420},
        wf::test::mockMatrix(2.0,9.0,80.0,80.0),
        wf::test::mockDistortion({1.0,2.0,3.0,4.0,5.0,6.0})
    );
    wf::CameraIntrinsics test_intrinsics_2_orig(
        {1920,1080},
        wf::test::mockMatrix(4.0,42.0,30.0,10.0),
        wf::test::mockDistortion({1.03,23.0,3.0,4.0,5.0})
    );

    wf::JSON test_intrinsics_0_jobject;
    wf::JSON test_intrinsics_1_jobject;
    wf::JSON test_intrinsics_2_jobject;

    auto jres0 = wf::CameraIntrinsics::toJSON(test_intrinsics_0_orig);
    ASSERT_TRUE(jres0);
    test_intrinsics_0_jobject = std::move(jres0.value());
    std::cout << "Test intrinsics 0 JSON: " << test_intrinsics_0_jobject.dump() << std::endl;

    auto jres1 = wf::CameraIntrinsics::toJSON(test_intrinsics_1_orig);
    ASSERT_TRUE(jres1);
    test_intrinsics_1_jobject = std::move(jres1.value());
    std::cout << "Test intrinsics 1 JSON: " << test_intrinsics_1_jobject.dump() << std::endl;

    auto jres2 = wf::CameraIntrinsics::toJSON(test_intrinsics_2_orig);
    ASSERT_TRUE(jres2);
    test_intrinsics_2_jobject = std::move(jres2.value());
    std::cout << "Test intrinsics 0 JSON: " << test_intrinsics_2_jobject.dump() << std::endl;

    auto dres0 = wf::CameraIntrinsics::fromJSON(test_intrinsics_0_jobject);
    ASSERT_TRUE(dres0);
    wf::CameraIntrinsics test_intrinsics_0_decode(std::move(dres0.value()));
    std::cout << "Test intrinsics 0 decode: " << test_intrinsics_0_decode.dump() << std::endl;

    auto dres1 = wf::CameraIntrinsics::fromJSON(test_intrinsics_1_jobject);
    ASSERT_TRUE(dres1);
    wf::CameraIntrinsics test_intrinsics_1_decode(std::move(dres1.value()));
    std::cout << "Test intrinsics 1 decode: " << test_intrinsics_1_decode.dump() << std::endl;

    auto dres2 = wf::CameraIntrinsics::fromJSON(test_intrinsics_2_jobject);
    ASSERT_TRUE(dres2);
    wf::CameraIntrinsics test_intrinsics_2_decode(std::move(dres2.value()));
    std::cout << "Test intrinsics 2 decode: " << test_intrinsics_2_decode.dump() << std::endl;

    wf::CameraConfiguration test_camconfig_orig(
        "test_camera",
        "/dev/video0",
        wf::CameraBackend::CSCORE,
        {30,{wf::ImageEncoding::YUYV,1920,1080}},
        {
            {wf::CamControl::EXPOSURE,"Exposure"},
            {wf::CamControl::BRIGHTNESS,"Brightness"}
        },
        {
            {
                {1920,1080},
                wf::test::mockMatrix(10.0,10.0,20.0,20.0),
                wf::test::mockDistortion({1.0,2.0,3.0,4.0,5.0})
            }
        },
        {
            {wf::CamControl::EXPOSURE,3}
        }
    );

    wf::JSON test_camconfig_jobject;
    auto ccjres = wf::CameraConfiguration::toJSON(test_camconfig_orig);
    ASSERT_TRUE(ccjres);
    test_camconfig_jobject = std::move(ccjres.value());
    std::cout << "Camera Configuration JSON: " << test_camconfig_jobject.dump() << std::endl;
    
    auto ccdres = wf::CameraConfiguration::fromJSON(test_camconfig_jobject);
    ASSERT_TRUE(ccdres);
    auto test_camconfig_decode = std::move(ccdres.value());
    std::cout << "Decoded Camera Configuration: " << test_camconfig_decode.dump() << std::endl;
    EXPECT_EQ(test_camconfig_jobject.dump(),test_camconfig_decode.dump());
    
}

TEST(JSONTests,validatorTest) {
    // Control, should pass validation
    std::string control_json
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "backend": "CSCORE",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "height": 720,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNESS": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": 979.1087360312252,
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                -0.005134231272255606,
                0.19101200082384226
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    // /backend is missing
    std::string bad_json_0
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "height": 720,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNESS": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": 979.1087360312252,
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                -0.005134231272255606,
                0.19101200082384226
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    // /format/frameFormat/height is missing
    std::string bad_json_1
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "backend": "CSCORE",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNESS": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": 979.1087360312252,
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                -0.005134231272255606,
                0.19101200082384226
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    // /calibrations/0/matrix/fx is a string
    std::string bad_json_2
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "backend": "CSCORE",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "height": 720,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNESS": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": "979.1087360312252",
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                -0.005134231272255606,
                0.19101200082384226
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    // /calibrations/0/distortion is 4 elements long
    std::string bad_json_3
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "backend": "CSCORE",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "height": 720,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNESS": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": 979.1087360312252,
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                -0.005134231272255606
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    // /calibrations/0/distortion/3 is a string
    std::string bad_json_4
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "backend": "CSCORE",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "height": 720,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNESS": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": 979.1087360312252,
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                "-0.005134231272255606",
                0.19101200082384226
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    // /backend is misspelled
    std::string bad_json_5
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "backend": "CSCOR",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "height": 720,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNESS": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": 979.1087360312252,
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                -0.005134231272255606,
                0.19101200082384226
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    // /controlAliases/SHARPNES is misspelled
    std::string bad_json_6
= R"({
    "nickname": "test_camera",
    "devpath": "/dev/video0",
    "backend": "CSCORE",
    "format": {
        "fps": 10,
        "frameFormat": {
            "width": 1280,
            "height": 720,
            "encoding": "YUYV"
        }
    },
    "controlAliases": {
        "BRIGHTNESS": "brightness",
        "CONTRAST": "contrast",
        "SATURATION": "saturation",
        "HUE": "hue",
        "AUTO_WHITE_BALANCE": "white_balance_automatic",
        "GAMMA": "gamma",
        "WHITE_BALANCE": "white_balance_temperature",
        "SHARPNES": "sharpness",
        "AUTO_EXPOSURE": "auto_exposure"
    },
    "calibrations": [
        {
            "resolution": {
                "width": 1280,
                "height": 720
            },
            "matrix": { 
                "fx": 979.1087360312252,
                "fy": 979.8457780935689,
                "cx": 608.5591334099096,
                "cy": 352.9815581130428
            },
            "distortion": [
                0.09581952042360092,
                -0.2603932345361037,
                0.0035795949814343524,
                -0.005134231272255606,
                0.19101200082384226
            ]
        }
    ],
    "controls": {
        "BRIGHTNESS": 128,
        "CONTRAST": 32,
        "AUTO_WHITE_BALANCE": 1
    }
})";
    wf::WFStatusResult (*validate)(const wf::JSON&) = wf::CameraConfiguration::validate;

    wf::JSON control_jobject = wf::JSON::parse(control_json);

    auto controlRes = wf::CameraConfiguration::validate(control_jobject);
    EXPECT_TRUE(controlRes);
    if (!controlRes) {
        std::cout << "Control error: " << controlRes.what() << std::endl;
    }

    wf::JSON bad_jobject_0 = wf::JSON::parse(bad_json_0);
    wf::JSON bad_jobject_1 = wf::JSON::parse(bad_json_1);
    wf::JSON bad_jobject_2 = wf::JSON::parse(bad_json_2);
    wf::JSON bad_jobject_3 = wf::JSON::parse(bad_json_3);
    wf::JSON bad_jobject_4 = wf::JSON::parse(bad_json_4);
    wf::JSON bad_jobject_5 = wf::JSON::parse(bad_json_5);
    wf::JSON bad_jobject_6 = wf::JSON::parse(bad_json_6);

    auto badRes0 = (*validate)(bad_jobject_0);
    EXPECT_FALSE(badRes0);
    std::cout << "Bad JSON 0 Error: " << badRes0.what() << std::endl;
    EXPECT_EQ("/backend: JSON_PROPERTY_NOT_FOUND",badRes0.what());
    EXPECT_EQ(wf::WFStatus::JSON_PROPERTY_NOT_FOUND,badRes0.status());

    auto badRes1 = (*validate)(bad_jobject_1);
    EXPECT_FALSE(badRes1);
    std::cout << "Bad JSON 1 Error: " << badRes1.what() << std::endl;
    EXPECT_EQ("/format/frameFormat/height: JSON_PROPERTY_NOT_FOUND",badRes1.what());
    EXPECT_EQ(wf::WFStatus::JSON_PROPERTY_NOT_FOUND,badRes1.status());

    auto badRes2 = (*validate)(bad_jobject_2);
    EXPECT_FALSE(badRes2);
    std::cout << "Bad JSON 2 Error: " << badRes2.what() << std::endl;
    EXPECT_EQ("/calibrations/0/matrix/fx: JSON_INVALID_TYPE",badRes2.what());
    EXPECT_EQ(wf::WFStatus::JSON_INVALID_TYPE,badRes2.status());

    auto badRes3 = (*validate)(bad_jobject_3);
    EXPECT_FALSE(badRes3);
    std::cout << "Bad JSON 3 Error: " << badRes3.what() << std::endl;
    EXPECT_EQ("/calibrations/0/distortion: JSON_SCHEMA_VIOLATION",badRes3.what());
    EXPECT_EQ(wf::WFStatus::JSON_SCHEMA_VIOLATION,badRes3.status());

    auto badRes4 = (*validate)(bad_jobject_4);
    EXPECT_FALSE(badRes4);
    std::cout << "Bad JSON 4 Error: " << badRes4.what() << std::endl;
    EXPECT_EQ("/calibrations/0/distortion/3: JSON_INVALID_TYPE",badRes4.what());
    EXPECT_EQ(wf::WFStatus::JSON_INVALID_TYPE,badRes4.status());

    auto badRes5 = (*validate)(bad_jobject_5);
    EXPECT_FALSE(badRes5);
    std::cout << "Bad JSON 5 Error: " << badRes5.what() << std::endl;
    EXPECT_EQ("/backend: JSON_SCHEMA_VIOLATION",badRes5.what());
    EXPECT_EQ(wf::WFStatus::JSON_SCHEMA_VIOLATION,badRes5.status());

    auto badRes6 = (*validate)(bad_jobject_6);
    EXPECT_FALSE(badRes6);
    std::cout << "Bad JSON 6 Error: " << badRes6.what() << std::endl;
    EXPECT_EQ("/controlAliases/SHARPNES: JSON_SCHEMA_VIOLATION",badRes6.what());
    EXPECT_EQ(wf::WFStatus::JSON_SCHEMA_VIOLATION,badRes6.status());


}