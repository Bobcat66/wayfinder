cmake_minimum_required(VERSION 3.25)
project(wayfinder-daemon CXX C)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE DAEMONSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/*.cpp"
)

add_executable(wayfinderd ${DAEMONSOURCES})
# Ensure the daemon depends on wfcore target (force ordering / visibility)
if(TARGET wfcore)
    add_dependencies(wayfinderd wfcore)
else()
    message(STATUS "Warning: wfcore target not available in this configuration")
endif()

# TODO: Figure out the library interface
target_link_libraries(wayfinderd
    PRIVATE
    wfcore
    httplib
)

target_include_directories(wayfinderd
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native
    ${CMAKE_CURRENT_BINARY_DIR}/../meta
)

install(TARGETS wayfinderd RUNTIME DESTINATION bin)




