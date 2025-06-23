#include "wfcore/video/processing/CVProcessPipe.h"
#include <stdexcept>
#include <type_traits>

namespace wf {

    template <CVImage T>
    template <std::same_as<CVProcessNode<T>>... Nodes>
    CVProcessPipe<T>::CVProcessPipe(Nodes&... nodes) {
        (this->nodes.push_back(std::ref(nodes)), ...);
        T* pad = &inpad;
        for (auto node : this->nodes) {
            node.setInpad(pad);
            inpad = &(node.getOutpad());
        }
        this->outpad = *pad;

        if constexpr (std::is_same_v<T,cv::Mat>()) {
            wrap = [](const T& in,Frame& out) {
                out.data = in;
            };
        } else if constexpr (std::is_same_v<T,cv::UMat>()){
            wrap = [](const T& in,Frame& out) {
                out.data = in.getMat();
            };
        }
    }

    template <CVImage T>
    void CVProcessPipe<T>::process() noexcept {
        for (auto& node : nodes) {
            node.process();
        }
    }

    template class CVProcessPipe<cv::Mat>;
    template class CVProcessPipe<cv::UMat>;
}