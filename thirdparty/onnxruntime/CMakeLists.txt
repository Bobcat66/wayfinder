cmake_minimum_required(VERSION 3.25)
project(thirdparty-onnxruntime)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# TODO: Add support for hardware accelerated EPs

FetchContent_Declare(
  onnxruntime
  GIT_REPOSITORY https://github.com/microsoft/onnxruntime.git
  GIT_TAG        main
  SOURCE_SUBDIR  v1.22.0
)
fetchcontent_makeavailable(onnxruntime)
