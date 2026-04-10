/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 Jesse Kane
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

#include "wfcore/inference/ObjectDetection.h"

namespace wf {

    wips_object_detection_t ObjectDetection::toWIPS_impl(const ObjectDetection& wfcore_object) {
        return {
            wfcore_object.objectClass,
            wfcore_object.confidence,
            wfcore_object.percentArea,
            wfcore_object.bboxTopLeftPixels.x,wfcore_object.bboxTopLeftPixels.y,
            wfcore_object.bboxBottomRightPixels.x,wfcore_object.bboxBottomRightPixels.y,
            wfcore_object.bboxTopLeftNorm.x,wfcore_object.bboxTopLeftNorm.y,
            wfcore_object.bboxBottomRightNorm.x,wfcore_object.bboxBottomRightNorm.y,
        };
    }

    ObjectDetection ObjectDetection::fromWIPS_impl(const wips_object_detection_t& wips_struct) {
        return {
            wips_struct.object_class,
            wips_struct.confidence,
            wips_struct.percent_area,
            {wips_struct.topleft_x_pixels,wips_struct.topleft_y_pixels},
            {wips_struct.bottomright_x_pixels,wips_struct.bottomright_y_pixels},
            {wips_struct.topleft_x_norm,wips_struct.topleft_y_norm},
            {wips_struct.bottomright_x_norm,wips_struct.bottomright_y_norm}
        };
    }

}