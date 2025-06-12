#pragma once

#include "wfcore/types.h"

namespace wf {
    /* 
     * Potential pipeline types to add:
     * Keypoint (pose estimation based on keypoints)
     * OptimizedApriltag (apriltag pose estimation based on a dynamic field optimized with a TagSLAM algorithm, requires a SLAM server somewhere on the network)
     * Depth (for depth cameras)
     * MonoSFM (local single camera Structure from Motion)
     * MonoSLAM (local single camera Simultaneous Mapping And Localization)
     */
    enum class PipelineType {
        Apriltag,
        ApriltagDetect, // This is for a pipeline that only detects apriltags, without solving PnP
        ObjDetect
    };

    class Pipeline {
        public:
        virtual PipelineResult process() const;
    };
}
