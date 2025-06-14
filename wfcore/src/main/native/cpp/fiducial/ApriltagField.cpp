#include "wfcore/fiducial/ApriltagField.h"

namespace wf {
    const Apriltag* ApriltagField::getTag(int id) const noexcept {
        auto it = aprilTags.find(id);
        if (it != aprilTags.end()) {
            return &it->second;
        }
        return nullptr; // Not found
    }
}