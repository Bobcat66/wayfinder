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

#include "wfcore/common/testutils.h"
#include <vector>

namespace wf::test {
    cv::Mat mockMatrix(double fx, double fy, double cx, double cy) {
        return (cv::Mat_<double>(3, 3) <<
            fx,  0, cx,
            0,  fy, cy,
            0,   0,  1
        );
    }
    cv::Mat mockDistortion(std::initializer_list<double> coeffs) {
        std::vector<double> covec;
        for (auto coeff : coeffs) {
            covec.push_back(coeff);
        }
        return cv::Mat(1, static_cast<int>(covec.size()), CV_64F, covec.data()).clone();
    }
}