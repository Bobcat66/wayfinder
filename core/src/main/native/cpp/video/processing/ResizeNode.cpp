#include "wfcore/video/processing/ResizeNode.h"
#include <opencv2/imgproc.hpp>
namespace wf {

    template <CVImage T>
    ResizeNode<T>::ResizeNode(const T& inpad_, int interpolator_, int outX_, int outY_) 
    : CVProcessNode<T>(inpad_) 
    , outsize(outX_,outY_)
    , interpolater(interpolator_) {}

    template <CVImage T>
    void ResizeNode<T>::process() noexcept {
        cv::resize(this->inpad,this->outpad,this->size,this->interpolater);
    }
}