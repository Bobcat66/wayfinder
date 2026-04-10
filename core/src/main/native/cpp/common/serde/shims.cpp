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

#include "wfcore/common/serde/shims.h"

namespace wf {
    gtsam::Pose3 pose3_wips2wfcore(const wips_pose3_t& pose) {
        return {
            {
                pose.wq,
                pose.xq,
                pose.yq,
                pose.zq
            },
            {
                pose.x,
                pose.y,
                pose.z
            }
        };
    }
    wips_pose3_t pose3_wfcore2wips(const gtsam::Pose3& pose) {
        auto q = pose.rotation().toQuaternion();
        wips_pose3_t wipspacket = {
            pose.x(),
            pose.y(),
            pose.z(),
            q.w(),
            q.x(),
            q.y(),
            q.z()
        };
        return wipspacket;
    }
    wips_blob_t* packPose3(const gtsam::Pose3& pose) {
        wips_pose3_t wipspose = pose3_wfcore2wips(pose);
        wips_blob_t* bin = wips_blob_create(sizeof(wips_pose3_t));
        wips_encode_pose3(bin, &wipspose);
        wips_pose3_free_resources(&wipspose);
        return bin;
    }
    gtsam::Pose3 unpackPose3(wips_blob_t* data) {
        wips_pose3_t wipspose;
        wips_decode_pose3(&wipspose,data);
        auto out = pose3_wips2wfcore(wipspose);
        wips_pose3_free_resources(&wipspose);
        return out;
    }
}