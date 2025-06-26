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

    int packPipelineResult(
        std::vector<uint8_t>& out,
        const PipelineResult& in
    ) noexcept {
        int bytesPacked = 0;
        uint64_t timestamp = in.captimeMicros;
        bytesPacked += packTrivial(out,timestamp);
        switch (in.type) {
            case (PipelineType::Apriltag):
                out.push_back((byte)0);
                bytesPacked++;
                bytesPacked += packTrivial(out,in.aprilTagPoses.size());
                for (auto atpose : in.aprilTagPoses) {
                    bytesPacked += packApriltagRelativePoseObservation(out,atpose);
                }
                if (in.cameraPose.has_value()) {
                    bytesPacked += packApriltagFieldPoseObservation(out,in.cameraPose.value());
                }
                break;
            case (PipelineType::ApriltagDetect):
                out.push_back((byte)1);
                // Not implemented yet
                break;
            case (PipelineType::ObjDetect):
                out.push_back((byte)2);
                // Not implemented yet
                break;
        }
        return bytesPacked;
    }

    PipelineResult unpackPipelineResult(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        double timestamp = unpackTrivial<double>(data,cit);
    }
}