cmake_minimum_required(VERSION 3.25)
project(thirdparty-cpp-httplib)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

fetchcontent_declare(
    cpp-httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.25.0
)
fetchcontent_makeavailable(cpp-httplib)