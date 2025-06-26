#include "wfcore/common/serde/serde.h"

namespace wf {
    int packApriltagFieldPoseObservation(
        std::vector<byte>& out,
        const ApriltagFieldPoseObservation& in
    ) noexcept {
        int bytesPacked = 0;
        bytesPacked += packTrivial(out,in.tagsUsed.size());
        for (int tagid : in.tagsUsed) {
            bytesPacked += packTrivial(out,tagid);
        }
        bytesPacked += packPose3(out,in.fieldPose0);
        bytesPacked += packTrivial(out,in.error0);
        if (in.tagsUsed.size() == 1) {
            bytesPacked += packPose3(out,in.fieldPose1.value());
            bytesPacked += packTrivial(out,in.error1.value());
        }
        return bytesPacked;
    }

    ApriltagFieldPoseObservation unpackApriltagFieldPoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        int tagsUsedSize = unpackTrivial<int>(data,cit);
        std::vector<int> tagsUsed;
        for (int i = 0; i < tagsUsedSize; i++) {
            tagsUsed.push_back(unpackTrivial<int>(data,cit));
        }
        auto fieldPose0 = unpackPose3(data,cit);
        double error0 = unpackTrivial<double>(data,cit);
        if (tagsUsedSize == 1) {
            auto fieldPose1 = unpackPose3(data,cit);
            double error1 = unpackTrivial<double>(data,cit);
            return {tagsUsed, fieldPose0, error0, fieldPose1, error1};
        } else {
            return {tagsUsed, fieldPose0, error0};
        }
    }

}