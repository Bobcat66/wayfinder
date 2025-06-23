#include "wfcore/common/marshalling/marshalling.h"

namespace wf {

    int marshalPipelineResult(std::vector<uint8_t>& out,const PipelineResult& in) noexcept {
        int bytesMarshalled = 0;
        uint64_t timestamp = in.captimeMicros;
        bytesMarshalled += marshalTrivial(out,timestamp);
        switch (in.type) {
            case (PipelineType::Apriltag):
                out.push_back((byte)0);
                bytesMarshalled++;
                bytesMarshalled += marshalTrivial(out,in.aprilTagPoses.size());
                for (auto atpose : in.aprilTagPoses) {
                    bytesMarshalled += marshalApriltagRelativePoseObservation(out,atpose);
                }
                if (in.cameraPose.has_value()) {
                    bytesMarshalled += marshalApriltagFieldPoseObservation(out,in.cameraPose.value());
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
        return bytesMarshalled;
    }

    int unmarshalPipelineResult(PipelineResult& out,const std::vector<uint8_t>& in) noexcept {
        return 0;
    }
}