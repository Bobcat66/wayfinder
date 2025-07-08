cmake_minimum_required(VERSION 3.25)
project(wayfinder-daemon CXX C)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE DAEMONSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/*.cpp"
)
add_executable(wayfinderd ${DAEMONSOURCES})

install(TARGETS wayfinderd RUNTIME DESTINATION bin)

# TODO: Figure out the library interface
target_link_libraries(wayfinderd
    PRIVATE
    wfcore
)

target_include_directories(wayfinderd
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native
    ${CMAKE_CURRENT_BINARY_DIR}/../meta
)



