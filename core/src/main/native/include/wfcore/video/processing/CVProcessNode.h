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

#include "wfcore/video/video_types.h"
#include "wfcore/video/video_utils.h"
namespace wf {
    template <CVImage T>
    class CVProcessNode {
    public:
        virtual ~CVProcessNode() = default;
        virtual void setInpad(const T& inpad) = 0;
        inline const T& getOutpad() const {return outpad;}
        inline FrameFormat getOutpadFormat() const {return getFormat(outpad);};
        virtual void process() noexcept = 0;
    protected:
        const T* inpad;
        T outpad;
    };
}