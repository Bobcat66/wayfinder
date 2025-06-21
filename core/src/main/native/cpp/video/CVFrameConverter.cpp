/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#include "wfcore/video/CVFrameConverter.h"

#include <opencv2/imgproc.hpp>

#include <stdexcept>

#include "wfcore/video/video_utils.h"

namespace wf {
    CVFrameConverter::CVFrameConverter(FrameFormat in, FrameFormat out)
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

    Frame CVFrameConverter::convert(const Frame& in) noexcept {
        if (in.data.size() != cv::Size(outFormat.cols, outFormat.rows)){
            cv::resize(in.data, buffer, cv::Size(outFormat.cols, outFormat.rows));
        } else {
            in.data.copyTo(buffer); // shallow copy if size matches
        }
        auto outmat = generateEmptyFrameBuf(outFormat);
        colorConverter(buffer,outmat);
        return Frame(in.captimeMicros,outFormat,outmat);
    }

}