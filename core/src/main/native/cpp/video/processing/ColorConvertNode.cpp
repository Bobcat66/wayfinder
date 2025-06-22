#include "wfcore/video/processing/ColorConvertNode.h"

#include <opencv2/imgproc.hpp>

#include <stdexcept>

namespace wf {
    template <CVImage T>
    ColorConvertNode<T>::ColorConvertNode(const T& inpad, ColorSpace inspace, ColorSpace outspace) : CVProcessNode<T>(inpad) {
        switch (inspace) {
            case COLOR:
                switch (outspace) {
                    case COLOR:
                        colorConverter = [](const T& in,T& out){
                            out = in;
                        };
                        break;
                    case GRAY:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2GRAY);
                        };
                        break;
                    case DEPTH:
                        // Expensive, don't use in the hot path
                        colorConverter = [](const T& in,T& out){
                            T tmp;
                            cv::cvtColor(in,tmp,cv::COLOR_BGR2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            case GRAY:
                switch (outspace) {
                    case COLOR:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2BGR);
                        };
                        break;
                    case GRAY:
                        colorConverter = [](const T& in,T& out){
                            out = in;
                        };
                        break;
                    case DEPTH:
                        colorConverter = [](const T& in,T& out){
                            in.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            case DEPTH:
                switch (outspace) {
                    case COLOR:
                        // Expensive, don't use in the hot path
                        colorConverter = [](const T& in,T& out){
                            T tmp;
                            cv::normalize(in, tmp, 0, 255, cv::NORM_MINMAX);
                            tmp.convertTo(tmp, CV_8UC1);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2BGR);
                        };
                        break;
                    case GRAY:
                        colorConverter = [](const T& in,T& out){
                            cv::normalize(in, out, 0, 255, cv::NORM_MINMAX);
                            out.convertTo(out, CV_8UC1);
                        };
                        break;
                    case DEPTH:
                        colorConverter = [](const T& in,T& out){
                            out = in;
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            default:
                throw std::invalid_argument("Attempted to convert from unknown colorspace");
        };

        int outcvformat;
        switch (outspace) {
            case COLOR:
                outcvformat = CV_8UC3;
                break;
            case GRAY:
                outcvformat = CV_8UC1;
                break;
            case DEPTH:
                outcvformat = CV_16UC1;
                break;
            default:
                throw std::invalid_argument("Attempted to convert from unknown colorspace");
        }
        this->outpad = T(inpad.rows,inpad.cols,outcvformat);
    }

    template <CVImage T>
    void ColorConvertNode<T>::process() noexcept {
        colorConverter(this->inpad,this->outpad);
    }

    template class ColorConvertNode<cv::Mat>;
    template class ColorConvertNode<cv::UMat>;
}
