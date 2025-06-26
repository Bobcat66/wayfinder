#include "wfcore/common/serde/serde.h"

namespace wf {

    int packPipelineResult(
        std::vector<uint8_t>& out,
        const PipelineResult& in
    ) noexcept {
        int bytesPacked = 0;
        uint64_t timestamp = in.captimeMicros;
        bytesPacked += packTrivial(out,timestamp);
        switch (in.type) {
            case (PipelineType::Apriltag):
                out.push_back((byte)0);
                bytesPacked++;
                bytesPacked += packTrivial(out,in.aprilTagPoses.size());
                for (auto atpose : in.aprilTagPoses) {
                    bytesPacked += packApriltagRelativePoseObservation(out,atpose);
                }
                if (in.cameraPose.has_value()) {
                    bytesPacked += packApriltagFieldPoseObservation(out,in.cameraPose.value());
                }
                break;
            case (PipelineType::ApriltagDetect):
                out.push_back((byte)1);
                // Not implemented yet
                break;
            case (PipelineType::ObjDetect):
                out.push_back((byte)2);
                // Not implemented yet
                break;
        }
        return bytesPacked;
    }

    PipelineResult unpackPipelineResult(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        double timestamp = unpackTrivial<double>(data,cit);
        
    }
}