cmake_minimum_required(VERSION 3.25)
project(wayfinder-core CXX C)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE CORESOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)
file(GLOB_RECURSE WIPSSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generated/wips/*.c"
)
add_library(wfcore ${CORESOURCES} ${WIPSSOURCES})

install(TARGETS wfcore LIBRARY DESTINATION lib)

if(WF_USE_EDGETPU)
    add_compile_definitions(WF_USE_EDGETPU)
endif()

if(WF_USE_CUDA)
    add_compile_definitions(WF_USE_CUDA)
endif()

if(WF_USE_HAILO)
    add_compile_definitions(WF_USE_HAILO)
endif()

if(WF_USE_ROCM)
    add_compile_definitions(WF_USE_ROCM)
endif()

if(WF_USE_RKNN)
    add_compile_definitions(WF_USE_RKNN)
endif()

if(WF_USE_COREML)
    add_compile_definitions(WF_USE_COREML)
endif()

if(WF_USE_OPENVINO)
    add_compile_definitions(WF_USE_OPENVINO)
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
    ${OpenCV_LIBS}
    gtsam
    spdlog::spdlog
    ntcore
    cscore
    PRIVATE
    apriltag
    nlohmann_json::nlohmann_json
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wfcore 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated
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

file(GLOB_RECURSE TESTSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test/native/*.cpp"
)
add_executable(wfcore_tests ${TESTSOURCES})

target_link_libraries(wfcore_tests 
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    wfcore
)

include(GoogleTest)
gtest_discover_tests(wfcore_tests)



