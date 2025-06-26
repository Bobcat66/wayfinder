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
    int packPose3(
        std::vector<uint8_t>& out,
        const gtsam::Pose3& in
    ) noexcept {
        int bytesPacked = 0;
        bytesPacked += packTrivial(out,in.x());
        bytesPacked += packTrivial(out,in.y());
        bytesPacked += packTrivial(out,in.z());
        const auto q = in.rotation().toQuaternion();
        bytesPacked += packTrivial(out, q.w());
        bytesPacked += packTrivial(out, q.x());
        bytesPacked += packTrivial(out, q.y());
        bytesPacked += packTrivial(out, q.z());
        return bytesPacked;
    }

    gtsam::Pose3 unpackPose3(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        double x = unpackTrivial<double>(data,cit);
        double y = unpackTrivial<double>(data,cit);
        double z = unpackTrivial<double>(data,cit);

        double wq = unpackTrivial<double>(data,cit);
        double xq = unpackTrivial<double>(data,cit);
        double yq = unpackTrivial<double>(data,cit);
        double zq = unpackTrivial<double>(data,cit);
        
        return {{wq,xq,yq,zq},{x,y,z}};
    }
}