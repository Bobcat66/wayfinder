#pragma once

#include "wfcore/pipeline/Pipeline.h"
#include <vector>
#include <cstdint>
#include <gtsam/geometry/Pose3.h>
#include "wfcore/common/serde/serde_utils.h"
#include "wfcore/pipeline/pnp.h"

namespace wf {

    int packPose3(std::vector<byte>& out,const gtsam::Pose3& in) noexcept;
    gtsam::Pose3 unpackPose3(
        const std::vector<byte>& in,
        std::vector<byte>::const_iterator cit
    ) noexcept;

    int packPipelineResult(std::vector<byte>& out,const PipelineResult& in) noexcept;
    PipelineResult unpackPipelineResult(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept;

    int packApriltagRelativePoseObservation(
        std::vector<byte>& out,
        const ApriltagRelativePoseObservation& in
    ) noexcept;
    ApriltagRelativePoseObservation unpackApriltagRelativePoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept;

    int packApriltagFieldPoseObservation(
        std::vector<byte>& out,
        const ApriltagFieldPoseObservation& in
    ) noexcept;
    ApriltagFieldPoseObservation unpackApriltagFieldPoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept;

}