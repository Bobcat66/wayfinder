cmake_minimum_required(VERSION 3.25)
project(wayfinder-common CXX C)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE CORESOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)
add_library(wfcommon ${CORESOURCES})

target_include_directories(wfcommon
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
)

enable_testing()

file(GLOB_RECURSE TESTSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test/native/*.cpp"
)
add_executable(run_tests ${TESTSOURCES})

target_link_libraries(run_tests 
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    wfcommon
)



