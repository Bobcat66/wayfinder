#include "wfcore/common/marshalling/marshalling.h"

namespace wf {
    int marshalApriltagRelativePoseObservation(
        std::vector<byte>& out,
        const ApriltagRelativePoseObservation& in
    ) noexcept {
        int bytesMarshalled = 0;
        bytesMarshalled += marshalTrivial(out,in.id);
        for (auto corner : in.corners) {
            bytesMarshalled += marshalTrivial(out,corner.x);
            bytesMarshalled += marshalTrivial(out,corner.y);
        }
        bytesMarshalled += marshalTrivial(out,in.decisionMargin);
        bytesMarshalled += marshalTrivial(out,in.hammingDistance);
        bytesMarshalled += marshalPose3(out,in.camPose0);
        bytesMarshalled += marshalTrivial(out,in.error0);
        bytesMarshalled += marshalPose3(out,in.camPose1);
        bytesMarshalled += marshalTrivial(out,in.error1);
        return bytesMarshalled;
    }

    int unmarshalApriltagRelativePoseObservation(
        const ApriltagRelativePoseObservation& out,
        const std::vector<byte>& in
    ) noexcept {
        return 0; // TODO: Finish this
    }
}