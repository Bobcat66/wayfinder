cmake_minimum_required(VERSION 3.25)
project(wayfinder-daemon CXX C)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE DAEMONSOURCES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/*.cpp"
)
add_executable(wayfinderd ${DAEMONSOURCES})

# TODO: Figure out the library interface
target_link_libraries(wayfinderd
    PRIVATE
    wfcore
) # In deployment builds, this will link statically, and there will be a cmake build type variable set in the top-level CMake file. I'll put that in eventually. For now, this is nowhere near deployment

target_include_directories(wayfinderd
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../meta
)



