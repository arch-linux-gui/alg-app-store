# Qt Widgets GUI layer. Links against `core` for package logic.
add_library(gui STATIC
    mainwindow.cpp
    mainwindow.h
    home_widget.cpp
    home_widget.h
    search_widget.cpp
    search_widget.h
    installed_widget.cpp
    installed_widget.h
    updates_widget.cpp
    updates_widget.h
    settings_widget.cpp
    settings_widget.h
    package_card.cpp
    package_card.h
    package_details_dialog.cpp
    package_details_dialog.h
    ${CMAKE_SOURCE_DIR}/resources.qrc
)

target_link_libraries(gui
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Concurrent
        core
        utils
)

target_compile_options(gui PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
