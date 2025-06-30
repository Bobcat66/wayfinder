#include "wfcore/inference/Tensorizer.h"

#include <cassert>
#include <cstring>

namespace wf {
    void Tensorizer::setTensorParameters(const TensorParameters& params) {
        if (params.channels != params.stds.size() || params.channels != params.means.size()) {
            throw std::invalid_argument("TensorParameters: Number of channels must match the size of stds and means.");
        }
        this->params = params;
        temp.create(params.height, params.width, CV_32FC(params.channels));
        channels.clear();
        channels.reserve(params.channels);
        for (int i = 0; i < params.channels; ++i) {
            channels.emplace_back(params.height, params.width, CV_32FC1);
        }
    }
    void Tensorizer::tensorize(const cv::Mat& input, float* output) const noexcept {
        input.convertTo(temp, CV_32FC, params.scale);
        cv::divide(temp, params.stds, temp);
        cv::subtract(temp, params.means, temp);
        
        if (!params.interleaved) {
            cv::split(temp, channels);
            for (int c = 0; c < params.channels; ++c) {
                for (int h = 0; h < params.height; ++h) {
                    const float* srcrow = channels[c].ptr<float>(h);
                    float* dstrow = output + (c * params.height * params.width) + h * params.width;
                    std::memcpy(dstrow, srcrow, params.width * sizeof(float));
                }
            }        
        } else {
            for (int h = 0; h < params.height; ++h) {
                const float* srcrow = temp.ptr<float>(h);
                float* dstrow = output + h * params.width * params.channels;
                std::memcpy(dstrow, srcrow, params.width * params.channels * sizeof(float));
            }
        }
    }
}