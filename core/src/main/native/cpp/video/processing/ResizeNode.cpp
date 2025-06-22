#include "wfcore/video/processing/ResizeNode.h"
#include <opencv2/imgproc.hpp>
namespace wf {

    template <CVImage T>
    ResizeNode<T>::ResizeNode(int interpolator_, int outX_, int outY_) 
    : outsize(outX_,outY_)
    , interpolater(interpolator_) {}

    template <CVImage T>
    void ResizeNode<T>::setInpad(const T& inpad) {
        this->inpad = &inpad;
        this->outpad = T(
            this->outsize.width,
            this->outsize.height,
            this->inpad->type()
        );
    }

    template <CVImage T>
    void ResizeNode<T>::process() noexcept {
        cv::resize(
            *(this->inpad),
            this->outpad,
            this->size,
            this->interpolater
        );
    }
}