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

#include "wfcore/pipeline/Pipeline.h"
#include "wfcore/inference/InferenceEngine.h"
#include "wfcore/video/video_types.h"
#include "wfcore/inference/Tensorizer.h"
#include "wfcore/hardware/CameraConfiguration.h"
#include "wfcore/pipeline/ObjectDetectionPipelineConfiguration.h"

#include <string>
#include <memory>
#include <vector>

namespace wf {

    class ObjectDetectionPipeline : public Pipeline {
    public:
        ObjectDetectionPipeline(ImageEncoding modelColorSpace_, std::unique_ptr<InferenceEngine> engine_, CameraIntrinsics intrinsics_);
        [[nodiscard]] 
        PipelineResult process(const cv::Mat& data, const FrameMetadata& meta) noexcept override;
        InferenceEngineType getEngineType() const noexcept { return engine->getEngineType(); }
        ModelArch getModelArch() const noexcept { return engine->getModelArch(); }
        ImageEncoding getModelColorSpace() const noexcept { };
        static std::unique_ptr<InferenceEngine> buildInferenceEngine(const ObjectDetectionPipelineConfiguration& config);
    private:
        void updatePostprocParams();
        std::unique_ptr<InferenceEngine> engine;
        ImageEncoding modelColorSpace;
        std::vector<RawBbox> bbox_buffer;
        std::vector<cv::Point2d> pixelCorner_buffer; // Even indexed points are topleft corners, odd indexed points are bottomright corners.
        std::vector<cv::Point2d> resizedPixelCorner_buffer;
        std::vector<cv::Point2d> normCorner_buffer;

        // This is for calculating corner angles, should be calibrated for the native input resolution
        CameraIntrinsics intrinsics;
        // Postproc params
        // These are dynamically calculated based on the tensor parameters and the native resolution,
        // essentially running the same algorithm used by the LetterboxNode to compute the padding and scale factors, but in reverse
        double horizontalShift; // -leftPadding
        double verticalShift; // -topPadding
        double scale;
    };
}
