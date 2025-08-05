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

#pragma once

#include <string>
#include <mutex>
#include "wfcore/common/status.h"
#include "wfcore/fiducial/tag_detector_configs.h"
#include "wfcore/fiducial/ApriltagConfiguration.h"
#include "wfcore/video/video_types.h"
#include "wfcore/inference/inference_configs.h"
#include <shared_mutex>
#include "wfcore/common/json_utils.h"

namespace wf {
    class WFDefaults {
    public:
        // Loads defaults from environment
        static WFStatusResult load(const JSON& jobject) {
            std::unique_lock lock(mtx_);
            return getInstance().load_impl(jobject);
        }

        static std::string tagField() {
            std::shared_lock lock(mtx_);
            return getInstance().tagField_;
        }

        static std::string tagFamily() {
            std::shared_lock lock(mtx_);
            return getInstance().tagFamily_;
        }

        static double tagSize() {
            std::shared_lock lock(mtx_);
            return getInstance().tagSize_;
        }

        static std::string modelFile() {
            std::shared_lock lock(mtx_);
            return getInstance().modelFile_;
        }

        static ApriltagDetectorConfig tagDetectorConfig() {
            std::shared_lock lock(mtx_);
            return getInstance().tagDetectorConfig_;
        }

        static QuadThresholdParams qtps() {
            std::shared_lock lock(mtx_);
            return getInstance().qtps_;
        }

        static TensorParameters tensorParameters() {
            std::shared_lock lock(mtx_);
            return getInstance().tensorParameters_;
        }

        static float nmsThreshold() {
            std::shared_lock lock(mtx_);
            return getInstance().nmsThreshold_;
        }

        static float confThreshold() {
            std::shared_lock lock(mtx_);
            return getInstance().confThreshold_;
        }

        static ImageEncoding modelColorSpace() {
            std::shared_lock lock(mtx_);
            return getInstance().modelColorSpace_;
        }

    private:
        static WFDefaults& getInstance() {
            static WFDefaults inst;
            return inst;
        }
        WFStatusResult load_impl(const JSON& jobject);

        WFDefaults();
        ~WFDefaults() = default;

        // Prevent copying and moving
        WFDefaults(const WFDefaults&) = delete;
        WFDefaults& operator=(const WFDefaults&) = delete;
        WFDefaults(WFDefaults&&) = delete;
        WFDefaults& operator=(WFDefaults&&) = delete;

        std::string tagField_;
        std::string tagFamily_;
        double tagSize_;
        std::string modelFile_;
        ApriltagDetectorConfig tagDetectorConfig_;
        QuadThresholdParams qtps_;
        TensorParameters tensorParameters_;
        InferenceEngineType engineType_;
        ModelArch modelArch_;
        ImageEncoding modelColorSpace_;
        float nmsThreshold_;
        float confThreshold_;

        static std::shared_mutex mtx_;
    };
}