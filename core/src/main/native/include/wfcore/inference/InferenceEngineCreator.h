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

#include "wfcore/inference/InferenceEngine.h"
#include <concepts>
#include <filesystem>
#include <memory>

namespace wf {

    template <typename T>
    concept HasCreatorImpl = requires (
        std::filesystem::path p,
        TensorParameters t,
        IEFilteringParams f
    ) {
        { T::creator_impl(p, t, f) } -> std::convertible_to<WFResult<std::unique_ptr<InferenceEngine>>>;
    };


    class InferenceEngineCreatorBase {
    public:
        virtual WFResult<std::unique_ptr<InferenceEngine>> operator()(
            std::filesystem::path modelPath,
            TensorParameters tensorParams,
            IEFilteringParams filterParams
        ) const = 0;
        virtual ~InferenceEngineCreatorBase() = default;
    };

    template <typename T>
        requires HasCreatorImpl<T>
    class InferenceEngineCreatorImpl : public InferenceEngineCreatorBase {
    public:
        WFResult<std::unique_ptr<InferenceEngine>> operator()(
            std::filesystem::path modelPath,
            TensorParameters tensorParams,
            IEFilteringParams filterParams
        ) const override {
            return T::creator_impl(std::move(modelPath),std::move(tensorParams),std::move(filterParams));
        }
    };

    class InferenceEngineCreator {
    public:
        template <typename T>
            requires HasCreatorImpl<T>
        InferenceEngineCreator() : self(std::make_shared<InferenceEngineCreatorImpl<T>>()) {}
        WFResult<std::unique_ptr<InferenceEngine>> operator()(
            std::filesystem::path modelPath,
            TensorParameters tensorParams,
            IEFilteringParams filterParams
        ) const {
            return (*self)(std::move(modelPath),std::move(tensorParams),std::move(filterParams));
        }

        template <typename T>
            requires HasCreatorImpl<T>
        static InferenceEngineCreator getCreator() {
            return { InferenceEngineCreator<T>() };
        }
    private:
        std::shared_ptr<const InferenceEngineCreatorBase> self;
    };
}