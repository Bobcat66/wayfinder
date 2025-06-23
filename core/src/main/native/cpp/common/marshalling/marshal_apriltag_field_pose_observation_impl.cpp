#include "wfcore/common/marshalling/marshalling.h"

namespace wf {
    int marshalApriltagFieldPoseObservation(
        std::vector<byte>& out,
        const ApriltagFieldPoseObservation& in
    ) noexcept {
        int bytesMarshalled = 0;
        bytesMarshalled += marshalTrivial(out,in.tagsUsed.size());
        for (int tagid : in.tagsUsed) {
            bytesMarshalled += marshalTrivial(out,tagid);
        }
        bytesMarshalled += marshalPose3(out,in.fieldPose0);
        bytesMarshalled += marshalTrivial(out,in.error0);
        if (in.fieldPose1.has_value()) {
            bytesMarshalled += marshalPose3(out,in.fieldPose1.value());
            bytesMarshalled += marshalTrivial(out,in.error1.value());
        }
        return bytesMarshalled;
    }

    int unmarshalApriltagFieldPoseObservation(
        ApriltagFieldPoseObservation& out,
        const std::vector<byte>& in
    ) noexcept {
        return 0;
    }

}