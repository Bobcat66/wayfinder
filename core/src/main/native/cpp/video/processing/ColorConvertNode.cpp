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
            case RGB565:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR5652RGB);
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR5652BGR);
                        };
                        break;
                    case RGB565:
                        colorConverter = [](const T& in,T& out){
                            out = in
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(out,in,cv::COLOR_BGR5652GRAY);
                        };
                        break;
                    case Y16:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_BGR5652GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case YUYV: throw std::runtime_error("RGB565 to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("RGB565 to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR5652BGRA);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR5652RGBA);
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
                        colorConverter = [](const T& in,T& out){
                            in.convertTo(out,CV_8UC1,1.0/256.0);
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
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2RGBA);
                        };
                        break;
                    case BGRA:
                        colorConverter - [this](const T& in, T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            in.convertTo(tmp,CV_8UC1,1.0/256.0);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2BGRA);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case YUYV:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2BGR_YUYV);
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2RGB_YUYV);
                        };
                        break;
                    case RGB565:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC3};
                            cv::cvtColor(in,tmp,cv::COLOR_YUV2RGB_YUYV);
                            cv::cvtColor(tmp,out,cv::COLOR_BGR2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2GRAY_YUYV);
                        };
                        break;
                    case Y16:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_YUV2GRAY_YUYV);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    // No conversions to YUYV or UYVY are supported in general, even from YUYV and UYVY.
                    // This is meant to prevent footguns where a convert node converting to YUYV breaks after switching to a different input encoding
                    case YUYV: throw std::runtime_error("YUYV to YUYV conversion is not supported at the moment. Why? Because screw you (yes, you in particular). That's why");
                    case UYVY: throw std::runtime_error("YUYV to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2RGBA_YUYV);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2BGRA_YUYV);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case UYVY:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2BGR_UYVY);
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2RGB_UYVY);
                        };
                        break;
                    case RGB565:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC3};
                            cv::cvtColor(in,tmp,cv::COLOR_YUV2RGB_UYVY);
                            cv::cvtColor(tmp,out,cv::COLOR_BGR2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2GRAY_UYVY);
                        };
                        break;
                    case Y16:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_YUV2GRAY_UYVY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    // No conversions to YUYV or UYVY are supported in general, even from YUYV and UYVY.
                    // This is meant to prevent footguns where a convert node converting to YUYV breaks after switching to a different input encoding
                    case YUYV: throw std::runtime_error("UYVY to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("UYVY to UYVY conversion is not supported at the moment. Why? Because screw you (yes, you in particular). That's why.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2RGBA_UYVY);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            cv::cvtColor(in,out,cv::COLOR_YUV2BGRA_UYVY);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case RGBA:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGBA2BGR);
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGBA2RGB);
                        };
                        break;
                    case RGB565:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGRA2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGBA2GRAY);
                        };
                        break;
                    case Y16:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_RGBA2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case YUYV: throw std::runtime_error("RGBA to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("RGBA to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            out = in;
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGBA2BGRA);
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case BGRA:
                switch (this->outcoding) {
                    case BGR24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGRA2BGR);
                        };
                        break;
                    case RGB24:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGRA2RGB);
                        };
                        break;
                    case RGB565:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGBA2BGR565);
                        };
                        break;
                    case Y8:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGRA2GRAY);
                        };
                        break;
                    case Y16:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_BGRA2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case YUYV: throw std::runtime_error("BGRA to YUYV conversion is not supported at the moment.");
                    case UYVY: throw std::runtime_error("BGRA to UYVY conversion is not supported at the moment.");
                    case RGBA:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGRA2RGBA);
                        };
                        break;
                    case BGRA:
                        colorConverter - [](const T& in, T& out){
                            out = in;
                        };
                        break;
                    case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
                    default: throw std::runtime_error("Attempted to convert from unknown encoding");
                }
                break;
            case MJPEG: throw std::runtime_error("Gng really thought the ColorConvertNode was a JPEG codec 💔💔💔");
            default:
                throw std::invalid_argument("Attempted to convert from unknown colorspace");
        };

    }

    template class ColorConvertNode<cv::Mat>;
    template class ColorConvertNode<cv::UMat>;
}
