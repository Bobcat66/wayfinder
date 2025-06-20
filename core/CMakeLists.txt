cmake_minimum_required(VERSION 3.25)
project(wayfinder-core CXX C)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE CORESOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)
add_library(wfcore ${CORESOURCES})

if(WF_CV_OPENCL)
    add_compile_definitions(WF_CV_OPENCL)
elseif(WF_CV_CUDA)
    add_compile_definitions(WF_CV_CUDA)
endif()

if(WF_USE_GSTREAMER)
    add_compile_definitions(WF_GSTREAMER)
    if(WF_USE_VIDEOLAN_CODECS)
        add_compile_definitions(WF_VIDEOLAN)
    endif()
endif()
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
    nlohmann_json::nlohmann_json
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wfcore 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
    ${OpenCV_INCLUDE_DIRS}
    PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/../meta
)

if(WF_USE_GSTREAMER)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
    pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)

    target_link_libraries(wfcore
        PRIVATE
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
    )
    target_include_directories(wfcore
        PRIVATE
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER_BASE_INCLUDE_DIRS}
    )
    
endif()

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



