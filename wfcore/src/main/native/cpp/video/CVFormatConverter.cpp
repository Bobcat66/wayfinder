#include "wfcore/video/CVFormatConverter.h"

#include <opencv2/imgproc.hpp>

#include <stdexcept>

#include "wfcore/video/video_utils.h"

namespace wf {
    CVFormatConverter::CVFormatConverter(FrameFormat in, FrameFormat out)
    : inFormat(std::move(in)), outFormat(std::move(out))
    , tmpFormat(inFormat.colorspace, outFormat.rows, outFormat.cols) {
        xscalefactor = out.cols / in.cols;
        yscalefactor = out.rows / in.rows;
        switch (in.colorspace) {
            case COLOR:
                switch (out.colorspace) {
                    case COLOR:
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            in.copyTo(out); // A deep copy is needed as the converter reuses preallocated buffers
                        };
                        break;
                    case GRAY:
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2GRAY);
                        };
                        break;
                    case DEPTH:
                        // Expensive, don't use in the hot path
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            cv::Mat tmp;
                            cv::cvtColor(in,tmp,cv::COLOR_BGR2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            case GRAY:
                switch (out.colorspace) {
                    case COLOR:
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2BGR);
                        };
                        break;
                    case GRAY:
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            in.copyTo(out);
                        };
                        break;
                    case DEPTH:
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            in.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            case DEPTH:
                switch (out.colorspace) {
                    case COLOR:
                        // Expensive, don't use in the hot path
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            cv::Mat tmp;
                            cv::normalize(in, tmp, 0, 255, cv::NORM_MINMAX);
                            tmp.convertTo(tmp, CV_8UC1);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2BGR);
                        };
                        break;
                    case GRAY:
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            cv::normalize(in, out, 0, 255, cv::NORM_MINMAX);
                            out.convertTo(out, CV_8UC1);
                        };
                        break;
                    case DEPTH:
                        colorConverter = [](cv::Mat& in,cv::Mat& out){
                            in.copyTo(out);
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            default:
                throw std::invalid_argument("Attempted to convert from unknown colorspace");
        };
        buffer = generateEmptyFrameBuf(tmpFormat);
    }

    void CVFormatConverter::convert(cv::Mat& in, cv::Mat& out) noexcept {
        if (in.size() != cv::Size(outFormat.cols, outFormat.rows)){
            cv::resize(in, buffer, cv::Size(outFormat.cols, outFormat.rows));
        } else {
            in.copyTo(buffer); // shallow copy if size matches
        }
        colorConverter(buffer,out);
    }

}