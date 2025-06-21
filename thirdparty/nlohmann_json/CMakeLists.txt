cmake_minimum_required(VERSION 3.25)
project(thirdparty-nlohmann_json)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

fetchcontent_declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
fetchcontent_makeavailable(json)