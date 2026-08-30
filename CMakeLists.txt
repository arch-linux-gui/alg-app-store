cmake_minimum_required(VERSION 3.16)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION_STRING)

project(explorer VERSION ${PROJECT_VERSION_STRING} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Concurrent
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ALPM REQUIRED libalpm)

add_subdirectory(src/utils)
add_subdirectory(src/core)
add_subdirectory(src/gui)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    gui
    core
    utils
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install desktop file
install(FILES assets/explorer.desktop
    DESTINATION share/applications
)

# Install icon
install(FILES assets/explorer.png
    DESTINATION share/pixmaps
)
