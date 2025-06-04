cmake_minimum_required(VERSION 3.25)
project(thirdparty-gtsam)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

set(GTSAM_BUILD_PYTHON OFF)
set(GTSAM_BUILD_UNSTABLE OFF)
set(GTSAM_ENABLE_BOOST_SERIALIZATION OFF)
set(GTSAM_USE_BOOST_FEATURES OFF)
set(GTSAM_BUILD_CONVENIENCE_LIBRARIES OFF)
set(GTSAM_BUILD_TESTS OFF)
set(GTSAM_BUILD_EXAMPLES_ALWAYS OFF)
set(GTSAM_BUILD_DOCS OFF)
set(GTSAM_CMAKE_BUILD_TYPE Release)
fetchcontent_declare(
    gtsam
    GIT_REPOSITORY    https://github.com/borglab/gtsam.git
    GIT_TAG           4.3a0
)
fetchcontent_makeavailable(apriltag)