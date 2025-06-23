#include "wfcore/common/marshalling/marshalling.h"

namespace wf {
    int marshalPose3(std::vector<uint8_t>& out,const gtsam::Pose3& in) noexcept {
        int bytesMarshalled = 0;
        bytesMarshalled += marshalTrivial(out,in.x());
        bytesMarshalled += marshalTrivial(out,in.y());
        bytesMarshalled += marshalTrivial(out,in.z());
        const auto q = in.rotation().toQuaternion();
        bytesMarshalled += marshalTrivial(out, q.w());
        bytesMarshalled += marshalTrivial(out, q.x());
        bytesMarshalled += marshalTrivial(out, q.y());
        bytesMarshalled += marshalTrivial(out, q.z());
        return bytesMarshalled;
    }

    int unmarshalPose3(gtsam::Pose3& out,const std::vector<uint8_t>& in) noexcept {
        return 0; // WIP
    }
}