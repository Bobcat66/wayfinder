cmake_minimum_required(VERSION 3.25)
project(wayfinder-demos CXX C)

file(GLOB WFCORE_DEMO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/wfcore/*.cpp")

foreach(WFCORE_DEMO_SRC ${WFCORE_DEMO_SOURCES})
    get_filename_component(WFCORE_DEMO_NAME ${WFCORE_DEMO_SRC} NAME_WE)
    add_executable(${WFCORE_DEMO_NAME} ${WFCORE_DEMO_SRC})
    target_link_libraries(${WFCORE_DEMO_NAME} wfcore)
endforeach()