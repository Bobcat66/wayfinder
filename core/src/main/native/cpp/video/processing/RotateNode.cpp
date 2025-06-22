#include "wfcore/video/processing/RotateNode.h"
#include <opencv2/imgproc.hpp>

namespace wf {

    template <CVImage T>
    RotateNode<T>::RotateNode(int rotation_) : rotation(rotation) {}

    template <CVImage T>
    void RotateNode<T>::process() noexcept {
        cv::rotate(*(this->inpad),this->rotation);
    }
}