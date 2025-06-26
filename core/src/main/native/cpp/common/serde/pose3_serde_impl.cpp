#include "wfcore/common/serde/serde.h"

namespace wf {
    int packPose3(
        std::vector<uint8_t>& out,
        const gtsam::Pose3& in
    ) noexcept {
        int bytesPacked = 0;
        bytesPacked += packTrivial(out,in.x());
        bytesPacked += packTrivial(out,in.y());
        bytesPacked += packTrivial(out,in.z());
        const auto q = in.rotation().toQuaternion();
        bytesPacked += packTrivial(out, q.w());
        bytesPacked += packTrivial(out, q.x());
        bytesPacked += packTrivial(out, q.y());
        bytesPacked += packTrivial(out, q.z());
        return bytesPacked;
    }

    gtsam::Pose3 unpackPose3(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        double x = unpackTrivial<double>(data,cit);
        double y = unpackTrivial<double>(data,cit);
        double z = unpackTrivial<double>(data,cit);

        double wq = unpackTrivial<double>(data,cit);
        double xq = unpackTrivial<double>(data,cit);
        double yq = unpackTrivial<double>(data,cit);
        double zq = unpackTrivial<double>(data,cit);
        
        return {{wq,xq,yq,zq},{x,y,z}};
    }
}