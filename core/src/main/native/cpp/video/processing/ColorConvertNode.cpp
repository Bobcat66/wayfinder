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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "wfcore/video/processing/ColorConvertNode.h"

#include <opencv2/imgproc.hpp>

#include <stdexcept>

namespace wf {
    template <CVImage T>
    ColorConvertNode<T>::ColorConvertNode(ColorSpace inspace, ColorSpace outspace) {
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
    }

    template <CVImage T>
    void ColorConvertNode<T>::setInpad(const T& inpad) {
        this->inpad = &inpad;
        this->outpad = T(this->inpad->rows,this->inpad->cols,this->outcvformat);
    }

    template <CVImage T>
    void ColorConvertNode<T>::process() noexcept {
        colorConverter(*(this->inpad),this->outpad);
    }

    template class ColorConvertNode<cv::Mat>;
    template class ColorConvertNode<cv::UMat>;
}
