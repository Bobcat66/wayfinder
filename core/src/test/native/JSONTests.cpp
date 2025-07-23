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

#include "wfcore/configuration/JSONLoader.h"
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
}