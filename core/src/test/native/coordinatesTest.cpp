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



#include <iostream>

#include "wfcore/utils/coordinates.h"
#include "wfcore/utils/units.h"

#include <gtest/gtest.h>

// Tests transforming OpenCV translations to WPILib translations
TEST(coordinatesTest, CVtoWPILIBTranslation){
    Eigen::Vector3d t_c(3.0,4.0,2.0);
    Eigen::Vector3d t_w_expected(2.0,-3.0,-4.0);
    auto t_w = wf::cvToWPILibCoords(t_c);
    EXPECT_TRUE(t_w.isApprox(t_w_expected));
}