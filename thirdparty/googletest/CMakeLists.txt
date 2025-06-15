cmake_minimum_required(VERSION 3.25)
project(thirdparty-googletest)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

set(GTEST_HAS_ABSL OFF)
fetchcontent_declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest
    GIT_TAG           v1.17.0
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
fetchcontent_makeavailable(googletest)