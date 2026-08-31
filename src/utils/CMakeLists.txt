# Shared utilities: spdlog-backed logging setup, shared types (header-only).
add_library(utils STATIC
    logging.cpp
    logging.h
    types.h
)

target_include_directories(utils PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Generate utils/version.h from the CMake-configured PROJECT_VERSION, which
# itself comes from the repo-root VERSION file — keeps a single source of
# truth instead of a hardcoded version string in application code.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/generated/utils/version.h
    @ONLY
)
target_include_directories(utils PUBLIC ${CMAKE_BINARY_DIR}/generated)

target_link_libraries(utils PUBLIC Qt6::Core spdlog::spdlog)

target_compile_options(utils PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
