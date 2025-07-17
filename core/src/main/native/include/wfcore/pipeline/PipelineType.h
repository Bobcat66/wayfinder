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

#pragma once
#include <cstdint>

namespace wf {
    /* 
     * Potential pipeline types to add:
     * Keypoint (pose estimation based on keypoints)
     * OptimizedApriltag (apriltag pose estimation based on a dynamic field optimized with a TagSLAM algorithm, requires a SLAM server somewhere on the network)
     * Depth (for depth cameras)
     * MonoSFM (local single camera Structure from Motion)
     * MonoSLAM (local single camera Simultaneous Mapping And Localization)
     */
    enum class PipelineType : uint8_t {
        NullType,
        Apriltag,
        ObjDetect
    };
}