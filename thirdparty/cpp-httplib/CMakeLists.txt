cmake_minimum_required(VERSION 3.25)
project(thirdparty-cpp-httplib)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
set(HTTPLIB_REQUIRE_ZSTD OFF)
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF)
fetchcontent_declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.25.0
)
fetchcontent_makeavailable(httplib)