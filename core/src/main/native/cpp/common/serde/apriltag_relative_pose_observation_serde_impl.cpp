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

#include "wfcore/common/serde/serde.h"

namespace wf {
    int packApriltagRelativePoseObservation(
        std::vector<byte>& out,
        const ApriltagRelativePoseObservation& in
    ) noexcept {
        int bytesPacked = 0;
        bytesPacked += packTrivial(out,in.id);
        for (auto corner : in.corners) {
            bytesPacked += packTrivial(out,corner.x);
            bytesPacked += packTrivial(out,corner.y);
        }
        bytesPacked += packTrivial(out,in.decisionMargin);
        bytesPacked += packTrivial(out,in.hammingDistance);
        bytesPacked += packPose3(out,in.camPose0);
        bytesPacked += packTrivial(out,in.error0);
        bytesPacked += packPose3(out,in.camPose1);
        bytesPacked += packTrivial(out,in.error1);
        return bytesPacked;
    }

    ApriltagRelativePoseObservation unpackApriltagRelativePoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        int id = unpackTrivial<int>(data,cit);
        std::vector<cv::Point2d> corners;
        for (int i = 0; i < 4; i++) {
            corners.emplace_back(
                unpackTrivial<double>(data,cit),
                unpackTrivial<double>(data,cit)
            );
        }
        double decisionMargin = unpackTrivial<double>(data,cit);
        double hammingDistance = unpackTrivial<double>(data,cit);
        auto camPose0 = unpackPose3(data,cit);
        double error0 = unpackTrivial<double>(data,cit);
        auto camPose1 = unpackPose3(data,cit);
        double error1 = unpackTrivial<double>(data,cit);

        return {id, corners, decisionMargin, hammingDistance, camPose0, error0, camPose1, error1};
    }
}