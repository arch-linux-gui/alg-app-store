# Core logic: libalpm wrapper, AUR client, package operations. No Qt Widgets
# dependency — kept pure Qt Core/Network so it stays usable headlessly and
# stays a viable seam for a future ALPM mock backend (see PLAN.md 0.5.x).
add_library(core STATIC
    alpm_wrapper.cpp
    alpm_wrapper.h
    aur_helper.cpp
    aur_helper.h
    package_manager.cpp
    package_manager.h
)

target_include_directories(core PUBLIC ${ALPM_INCLUDE_DIRS})
target_link_directories(core PUBLIC ${ALPM_LIBRARY_DIRS})

target_link_libraries(core
    PUBLIC
        Qt6::Core
        Qt6::Network
        ${ALPM_LIBRARIES}
        utils
)

target_compile_options(core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
