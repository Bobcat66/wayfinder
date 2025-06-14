cmake_minimum_required(VERSION 3.25)
project(wayfinder-core CXX C)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE CORESOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)
add_library(wfcore ${CORESOURCES})

# TODO: Figure out the library interface
target_link_libraries(wfcore 
    PUBLIC
    gtsam
    ${OpenCV_LIBS}
    spdlog::spdlog
    PRIVATE
    ntcore
    cscore
    apriltag
    wfvtk
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wfcore 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
    ${OpenCV_INCLUDE_DIRS}
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
    wfcore
)



