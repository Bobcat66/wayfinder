// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Point2.h>

namespace wf {
    
    // Applies a translation to a pose in the pose frame
    inline gtsam::Pose3 translatePoseFrame(const gtsam::Pose3& pose, double x, double y, double z) {
        // Transform the pose by the frame
        gtsam::Pose3 translation = gtsam::Pose3(gtsam::Rot3::Identity(), gtsam::Point3(x, y, z));
        return pose.compose(translation);
    }

    inline gtsam::Pose3 translatePoseFrame(const gtsam::Pose3& pose, const gtsam::Point3& translation) {
        // Transform the pose by the frame
        return translatePoseFrame(pose, translation.x(), translation.y(), translation.z());
    }

    inline gtsam::Pose2 translatePoseFrame(const gtsam::Pose2& pose, double x, double y) {
        // Transform the pose by the frame
        gtsam::Pose2 translation = gtsam::Pose2(gtsam::Rot2::Identity(), gtsam::Point2(x, y));
        return pose.compose(translation);
    }

    inline gtsam::Pose2 translatePoseFrame(const gtsam::Pose2& pose, const gtsam::Point2& translation) {
        // Transform the pose by the frame
        return translatePoseFrame(pose, translation.x(), translation.y());
    }
}