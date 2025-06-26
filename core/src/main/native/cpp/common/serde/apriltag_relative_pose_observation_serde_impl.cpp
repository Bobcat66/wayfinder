#include "wfcore/common/serde/serde.h"

namespace wf {
    int packApriltagRelativePoseObservation(
        std::vector<byte>& out,
        const ApriltagRelativePoseObservation& in
    ) noexcept {
        int bytesPacked = 0;
        bytesPacked += packTrivial(out,in.id);
        for (auto corner : in.corners) {
            bytesPacked += packTrivial(out,corner.x);
            bytesPacked += packTrivial(out,corner.y);
        }
        bytesPacked += packTrivial(out,in.decisionMargin);
        bytesPacked += packTrivial(out,in.hammingDistance);
        bytesPacked += packPose3(out,in.camPose0);
        bytesPacked += packTrivial(out,in.error0);
        bytesPacked += packPose3(out,in.camPose1);
        bytesPacked += packTrivial(out,in.error1);
        return bytesPacked;
    }

    ApriltagRelativePoseObservation unpackApriltagRelativePoseObservation(
        const std::vector<byte>& data,
        std::vector<byte>::const_iterator cit
    ) noexcept {
        int id = unpackTrivial<int>(data,cit);
        std::vector<cv::Point2d> corners;
        for (int i = 0; i < 4; i++) {
            corners.emplace_back(
                unpackTrivial<double>(data,cit),
                unpackTrivial<double>(data,cit)
            );
        }
        double decisionMargin = unpackTrivial<double>(data,cit);
        double hammingDistance = unpackTrivial<double>(data,cit);
        auto camPose0 = unpackPose3(data,cit);
        double error0 = unpackTrivial<double>(data,cit);
        auto camPose1 = unpackPose3(data,cit);
        double error1 = unpackTrivial<double>(data,cit);

        return {id, corners, decisionMargin, hammingDistance, camPose0, error0, camPose1, error1};
    }
}