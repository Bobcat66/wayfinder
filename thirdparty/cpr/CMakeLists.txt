cmake_minimum_required(VERSION 3.25)
project(thirdparty-cpr)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

fetchcontent_declare(
    cpr
    GIT_REPOSITORY    https://github.com/libcpr/cpr.git
    GIT_TAG           1.12.0
)

fetchcontent_makeavailable(cpr)