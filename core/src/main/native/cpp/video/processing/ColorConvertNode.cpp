/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "wfcore/video/processing/ColorConvertNode.h"

#include <opencv2/imgproc.hpp>

#include <stdexcept>
#include <cassert>

namespace wf {
    template <CVImage T>
    ColorConvertNode<T>::ColorConvertNode(ImageEncoding outcoding_) {
        this->outcoding = outcoding_;
        updateColorConverter();
    }

    template <CVImage T>
    void ColorConvertNode<T>::updateBuffers() {
        this->outpad = T{
            this->inpad->format.rows,
            this->inpad->format.cols,
            getCVTypeFromEncoding(this->outcoding)
        };
        updateColorConverter();
    }

    template <CVImage T>
    void ColorConvertNode<T>::process() noexcept {
        colorConverter(*(this->inpad),this->outpad);
    }

    template <CVImage T>
    void ColorConvertNode<T>::updateColorConverter() {
        switch (*(this->incoding)) {
            case BGR24:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            out = in;
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2RGB);
                        };
                        break;
                    case RGB565:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGB2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2GRAY);
                        };
                        break;
                    case Y16:
                        // Expensive, don't use in the hot path
                        colorConverter = [](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_BGR2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case YUYV: throw std::runtime_error("BGR24 to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("BGR24 to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2RGBA);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2BGRA);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case RGB24:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGB2BGR)l
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            out = in;
                        };
                        break;
                    case RGB565:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGB2GRAY);
                        };
                        break;
                    case Y16:
                        // Expensive, don't use in the hot path
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_RGB2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case YUYV: throw std::runtime_error("RGB24 to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("RGB24 to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGB2RGBA);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGB2BGRA);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case Y8:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2BGR);
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2RGB);
                        };
                        break;
                    case RGB565:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            out = in
                        };
                        break;
                    case Y16:
                        colorConverter = [](const T& in,T& out){
                            in.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case YUYV: throw std::runtime_error("Y8 to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("Y8 to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2RGBA);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2BGRA);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case Y16:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2BGR);
                        };
                        break;
                    case RGB24:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2RGB);
                        };
                        break;
                    case RGB565:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            out = tmp
                        };
                        break;
                    case Y16:
                        colorConverter = [](const T& in,T& out){
                            out = in
                        };
                        break;
                    case YUYV: throw std::runtime_error("Y16 to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("Y16 to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2RGBA);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2BGRA);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            default:
                throw std::invalid_argument("Attempted to convert from unknown colorspace");
        };

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
            case RGB:
                outcvformat = CV_8UC3;
                break;
            default:
                throw std::invalid_argument("Attempted to convert from unknown colorspace");
                
        }

    }

    template class ColorConvertNode<cv::Mat>;
    template class ColorConvertNode<cv::UMat>;
}
