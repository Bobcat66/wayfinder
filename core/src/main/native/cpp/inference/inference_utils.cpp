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


#include "wfcore/inference/inference_utils.h"

namespace wf {
    std::array<cv::Point2d, 4> getCornersYOLO(float* yolo_result) {
        double xcenter = yolo_result[0];
        double ycenter = yolo_result[1];
        double width = yolo_result[2];
        double height = yolo_result[3];
        return {
            cv::Point2d{xcenter - (width/2),ycenter - (height/2)},
            cv::Point2d{xcenter + (width/2),ycenter - (height/2)},
            cv::Point2d{xcenter + (width/2),ycenter + (height/2)},
            cv::Point2d{xcenter - (width/2),ycenter + (height/2)}
        };
    }
    double getConfidenceYOLO(float* yolo_result, int obj_class) {
        double objectness = yolo_result[4];
        return objectness * yolo_result[5 + obj_class];
    }
    int getClassYOLO(float* yolo_result, int num_classes) {
        double highest_confidence = yolo_result[5];
        double obj_class = 0; 
        for (int i = 1; i < num_classes; ++i) {
            if (yolo_result[5 + i] > highest_confidence) {
                obj_class = i;
                highest_confidence = yolo_result[5 + i];
            }
        }
        return obj_class;
    }
}