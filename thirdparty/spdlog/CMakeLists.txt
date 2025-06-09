cmake_minimum_required(VERSION 3.25)
project(thirdparty-spdlog)


include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

fetchcontent_declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)
fetchcontent_makeavailable(spdlog)