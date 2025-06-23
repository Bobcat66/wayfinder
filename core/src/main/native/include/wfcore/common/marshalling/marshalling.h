#pragma once

#include "wfcore/pipeline/Pipeline.h"
#include <vector>
#include <cstdint>
#include <gtsam/geometry/Pose3.h>
#include "wfcore/common/marshalling/marshalling_utils.h"
#include "wfcore/pipeline/pnp.h"

namespace wf {

    int marshalPose3(std::vector<byte>& out,const gtsam::Pose3& in) noexcept;
    int unmarshalPose3(gtsam::Pose3& out,const std::vector<byte>& in) noexcept;

    int marshalPipelineResult(std::vector<byte>& out,const PipelineResult& in) noexcept;
    int unmarshalPipelineResult(PipelineResult& out,const std::vector<byte>& in) noexcept;

    int marshalApriltagRelativePoseObservation(
        std::vector<byte>& out,
        const ApriltagRelativePoseObservation& in
    ) noexcept;
    int unmarshalApriltagRelativePoseObservation(
        const ApriltagRelativePoseObservation& out,
        const std::vector<byte>& in
    ) noexcept;

    int marshalApriltagFieldPoseObservation(
        std::vector<byte>& out,
        const ApriltagFieldPoseObservation& in
    ) noexcept;
    int unmarshalApriltagFieldPoseObservation(
        ApriltagFieldPoseObservation& out,
        const std::vector<byte>& in
    ) noexcept;

}