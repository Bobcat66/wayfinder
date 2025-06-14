cmake_minimum_required(VERSION 3.25)
project(wfvtk CXX C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)

file(GLOB_RECURSE CSOURCES
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/c/*.c"
)

file(GLOB_RECURSE CXXSOURCES
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/cpp/*.cpp"
)

add_library(wfvtk ${CSOURCES} ${CXXSOURCES})

# TODO: Figure out the library interface
target_link_libraries(wfvtk
    PRIVATE
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_BASE_LIBRARIES}
)

target_include_directories(wfvtk
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include
    PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_BASE_INCLUDE_DIRS}
)