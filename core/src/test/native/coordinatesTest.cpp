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



#include <iostream>

#include "wfcore/utils/coordinates.h"
#include "wfcore/utils/units.h"

#include <gtest/gtest.h>

static bool posesAreApproxEqual(const gtsam::Pose3& p1, const gtsam::Pose3& p2, 
                         double tol_rotation = 1e-6, double tol_translation = 1e-6) {
    // Rotation difference: use localCoordinates (log map) norm
    gtsam::Vector6 diff = p1.localCoordinates(p2); // 6D vector: rotation(3), translation(3)

    // Rotation components
    gtsam::Vector3 rot_diff = diff.head<3>();
    // Translation components
    gtsam::Vector3 trans_diff = diff.tail<3>();

    return (rot_diff.norm() < tol_rotation) && (trans_diff.norm() < tol_translation);
}
// Tests transforming OpenCV translations to WPILib translations
TEST(coordinatesTest, CVtoWPILIBTranslation){
    Eigen::Vector3d t_c(3.0,4.0,2.0);
    Eigen::Vector3d t_w_expected(2.0,-3.0,-4.0);
    auto t_w = wf::cvToWPILibCoords(t_c);
    EXPECT_TRUE(t_w.isApprox(t_w_expected));
    gtsam::Rot3 rotation = gtsam::Rot3::RzRyRx(1, 2, 3); // radians
    gtsam::Point3 translation(100, 50, 20);
    gtsam::Pose3 orig_pose(rotation,translation);
    cv::Mat tvec;
    cv::Mat rvec;
    wf::WPILibPose3ToCvPoseVecs(orig_pose,rvec,tvec);
    auto out_pose = wf::cvPoseVecsToWPILibPose3(rvec,tvec);
    EXPECT_TRUE(posesAreApproxEqual(orig_pose,out_pose));
}