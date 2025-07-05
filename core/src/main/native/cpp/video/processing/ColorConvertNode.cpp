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
    ColorConvertNode<T>::ColorConvertNode(ColorSpace inspace_, ColorSpace outspace_) 
    : inspace(inspace_), outspace(outspace_) {
        updateColorConverter();
    }

    template <CVImage T>
    void ColorConvertNode<T>::updateBuffers() {
        this->outpad = T(this->inpad->rows,this->inpad->cols,this->outcvformat);
        updateColorConverter();
    }

    template <CVImage T>
    void ColorConvertNode<T>::process() noexcept {
        colorConverter(*(this->inpad),this->outpad);
    }

    template <CVImage T>
    void ColorConvertNode<T>::updateColorConverter() {
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
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_BGR2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case RGB:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_BGR2RGB);
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
                    case RGB:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_GRAY2RGB);
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            case DEPTH:
                switch (outspace) {
                    case COLOR:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::normalize(in, tmp, 0, 255, cv::NORM_MINMAX,CV_8U);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2BGR);
                        };
                        break;
                    case GRAY:
                        colorConverter = [](const T& in,T& out){
                            cv::normalize(in, out, 0, 255, cv::NORM_MINMAX, CV_8U);
                        };
                        break;
                    case DEPTH:
                        colorConverter = [](const T& in,T& out){
                            out = in;
                        };
                        break;
                    case RGB:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::normalize(in, tmp, 0, 255, cv::NORM_MINMAX, CV_8U);
                            cv::cvtColor(tmp,out,cv::COLOR_GRAY2RGB);
                        };
                        break;
                    default:
                        throw std::invalid_argument("Attempted to convert from unknown colorspace");
                }
                break;
            case RGB:
                switch (outspace) {
                    case COLOR:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGB2BGR);
                        };
                        break;
                    case GRAY:
                        colorConverter = [](const T& in,T& out){
                            cv::cvtColor(in,out,cv::COLOR_RGB2GRAY);
                        };
                        break;
                    case DEPTH:
                        colorConverter = [this](const T& in,T& out){
                            static T tmp{this->getInpad()->rows,this->getInpad()->cols,CV_8UC1};
                            cv::cvtColor(in,tmp,cv::COLOR_RGB2GRAY);
                            tmp.convertTo(out,CV_16UC1, 256.0);
                        };
                        break;
                    case RGB:
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
