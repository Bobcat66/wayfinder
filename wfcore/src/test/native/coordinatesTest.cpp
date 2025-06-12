#include <iostream>

#include "utils/coordinates.h"
#include "utils/units.h"

#include <gtest/gtest.h>

// Tests transforming OpenCV translations to WPILib translations
TEST(coordinatesTest, CVtoWPILIBTranslation){
    Eigen::Vector3d t_c(3.0,4.0,2.0);
    Eigen::Vector3d t_w_expected(2.0,-3.0,-4.0);
    auto t_w = wf::cvToWPILibCoords(t_c);
    EXPECT_TRUE(t_w.isApprox(t_w_expected));
}