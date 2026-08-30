# Header-only utilities (logger, shared types). No sources to compile.
add_library(utils INTERFACE)

target_include_directories(utils INTERFACE ${CMAKE_SOURCE_DIR}/src)

# Generate utils/version.h from the CMake-configured PROJECT_VERSION, which
# itself comes from the repo-root VERSION file — keeps a single source of
# truth instead of a hardcoded version string in application code.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/generated/utils/version.h
    @ONLY
)
target_include_directories(utils INTERFACE ${CMAKE_BINARY_DIR}/generated)

target_link_libraries(utils INTERFACE Qt6::Core)
