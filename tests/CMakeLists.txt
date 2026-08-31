# Pure-logic unit tests only (see PLAN.md 0.3.5): AlpmWrapper/PackageManager
# remain hard singletons touching real libalpm/pkexec and are not covered
# here. A full ALPM mock backend is deferred to 0.5.x.
add_executable(explorer_tests
    test_aur_helper.cpp
    test_pacman_conf.cpp
    test_progress_parser.cpp
)

target_link_libraries(explorer_tests PRIVATE
    core
    Catch2::Catch2WithMain
)

target_compile_options(explorer_tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

include(Catch)
catch_discover_tests(explorer_tests)
