// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wfcore/pipeline/ApriltagPipeline.h"
#include "wfcore/configuration/configurations.h"
#include <algorithm>
#include <wfcore/pipeline/pnp.h>

namespace wf {
    ApriltagPipeline::ApriltagPipeline(ApriltagPipelineConfiguration config_, CameraIntrinsics intrinsics_, ApriltagField field_)
    : config(std::move(config_)), intrinsics(std::move(intrinsics_)), field(field_) {
        detector.setQuadThresholdParams(config.detQTPs);
        detector.setConfig(config.detConfig);
        detector.addFamily(field.tagFamily);
    }

    PipelineResult ApriltagPipeline::process(const Frame& frame) const noexcept {
        auto detections = detector.detect(frame.data);
        std::erase_if(detections, [this](ApriltagDetection detection) {
            return this->config.detectorExcludes.contains(detection.id);
        });
        std::vector<ApriltagRelativePoseObservation> atagPoses;
        if (config.solveTagRelative) {
            for (auto detection : detections) {
                auto atagPose = solvePNPApriltagRelative(
                    detection,
                    field,
                    intrinsics
                );
                if (atagPose.has_value()) {
                    atagPoses.push_back(atagPose.value());
                }
            }
        }
        auto fieldPose = solvePNPApriltag(
            detections,
            field,
            intrinsics,
            config.SolvePNPExcludes
        );
        return PipelineResult::ApriltagPipelineResult(
            atagPoses,
            fieldPose,
            frame
        );
    }

}

