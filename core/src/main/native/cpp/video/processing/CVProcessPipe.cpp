#include "wfcore/video/processing/CVProcessPipe.h"

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
    }

    template <CVImage T>
    void CVProcessPipe<T>::process() noexcept {
        for (auto node : nodes) {
            node.process();
        }
    }
}