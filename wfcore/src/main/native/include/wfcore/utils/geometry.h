// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Point2.h>
#include <Eigen/Core>

namespace wf {

    namespace constants {  
        inline constexpr double pi = 3.141592653589793;
    }
    
    // Applies a translation to a pose in the pose frame
    inline gtsam::Pose3 translatePoseFrame(const gtsam::Pose3& pose, const gtsam::Point3& translation) {
        // Transform the pose by the frame
        gtsam::Pose3 transform = gtsam::Pose3(gtsam::Rot3::Identity(), translation);
        return pose.compose(transform);
    }

    inline gtsam::Pose3 translatePoseFrame(const gtsam::Pose3& pose, double x, double y, double z) {
        // Transform the pose by the frame
        return translatePoseFrame(pose, gtsam::Point3(x, y, z));
    }

    inline gtsam::Pose2 translatePoseFrame(const gtsam::Pose2& pose, const gtsam::Point2& translation) {
        // Transform the pose by the frame
        gtsam::Pose2 transform = gtsam::Pose2(gtsam::Rot2::Identity(), translation);
        return pose.compose(transform);
    }

    inline gtsam::Pose2 translatePoseFrame(const gtsam::Pose2& pose, double x, double y) {
        // Transform the pose by the frame
        return translatePoseFrame(pose, gtsam::Point2(x, y));
    }
}