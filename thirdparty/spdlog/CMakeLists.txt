cmake_minimum_required(VERSION 3.25)
project(thirdparty-spdlog)


include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# Use compiled spdlog, not header-only
set(SPDLOG_HEADER_ONLY OFF CACHE BOOL "Build shared spdlog" FORCE)
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Build shared spdlog" FORCE)
set(SPDLOG_BUILD_STATIC OFF CACHE BOOL "" FORCE)
fetchcontent_declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)
fetchcontent_makeavailable(spdlog)

# --- Install the built shared library ---
if(TARGET spdlog)
    # Ensure the library is built before installation
    install(TARGETS spdlog
        EXPORT spdlogTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
else()
    message(FATAL_ERROR "spdlog target not found after FetchContent.")
endif()
