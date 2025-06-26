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
    int packApriltagFieldPoseObservation(
        std::vector<byte>& out,
        const ApriltagFieldPoseObservation& in
    ) noexcept {
        int bytesPacked = 0;
        bytesPacked += packTrivial(out,in.tagsUsed.size());
        for (int tagid : in.tagsUsed) {
            bytesPacked += packTrivial(out,tagid);
        }
        bytesPacked += packPose3(out,in.fieldPose0);
        bytesPacked += packTrivial(out,in.error0);
        if (in.tagsUsed.size() == 1) {
            bytesPacked += packPose3(out,in.fieldPose1.value());
            bytesPacked += packTrivial(out,in.error1.value());
        }
        return bytesPacked;
    }

    ApriltagFieldPoseObservation unpackApriltagFieldPoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        int tagsUsedSize = unpackTrivial<int>(data,cit);
        std::vector<int> tagsUsed;
        for (int i = 0; i < tagsUsedSize; i++) {
            tagsUsed.push_back(unpackTrivial<int>(data,cit));
        }
        auto fieldPose0 = unpackPose3(data,cit);
        double error0 = unpackTrivial<double>(data,cit);
        if (tagsUsedSize == 1) {
            auto fieldPose1 = unpackPose3(data,cit);
            double error1 = unpackTrivial<double>(data,cit);
            return {tagsUsed, fieldPose0, error0, fieldPose1, error1};
        } else {
            return {tagsUsed, fieldPose0, error0};
        }
    }

}