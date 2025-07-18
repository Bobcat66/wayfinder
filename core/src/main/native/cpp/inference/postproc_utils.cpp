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


#include "wfcore/inference/postproc_utils.h"

namespace wf {
    
    [[ deprecated ]]
    std::array<cv::Point2f, 4> getCorners_CVRect2d(const cv::Rect2d& bbox) {
        float bboxf_x = static_cast<float>(bbox.x);
        float bboxf_y = static_cast<float>(bbox.y);
        float bboxf_width = static_cast<float>(bbox.width);
        float bboxf_height = static_cast<float>(bbox.height);
        return {
            cv::Point2f{bboxf_x, bboxf_y},
            cv::Point2f{bboxf_x + bboxf_width, bboxf_y},
            cv::Point2f{bboxf_x + bboxf_width, bboxf_y + bboxf_height},
            cv::Point2f{bboxf_x, bboxf_y + bboxf_height}
        };
    }

    [[ deprecated ]]
    std::array<cv::Point2f, 4> getCorners_CVRect2f(const cv::Rect2f& bbox) {
        return {
            cv::Point2f{bbox.x, bbox.y},
            cv::Point2f{bbox.x + bbox.width, bbox.y},
            cv::Point2f{bbox.x + bbox.width, bbox.y + bbox.height},
            cv::Point2f{bbox.x, bbox.y + bbox.height}
        };
    }

    std::array<cv::Point2f, 4> getCorners_RawBbox(const RawBbox& bbox) {
        return {
            cv::Point2f{bbox.x, bbox.y},
            cv::Point2f{bbox.x + bbox.width, bbox.y},
            cv::Point2f{bbox.x + bbox.width, bbox.y + bbox.height},
            cv::Point2f{bbox.x, bbox.y + bbox.height}
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