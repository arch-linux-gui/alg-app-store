cmake_minimum_required(VERSION 3.16)

project(alg-app-store VERSION 0.2.30 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Concurrent
)

# Find libalpm
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALPM REQUIRED libalpm)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${ALPM_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    
    # Core
    src/core/alpm_wrapper.cpp
    src/core/aur_helper.cpp
    src/core/package_manager.cpp
    
    # GUI
    src/gui/mainwindow.cpp
    src/gui/home_widget.cpp
    src/gui/search_widget.cpp
    src/gui/installed_widget.cpp
    src/gui/updates_widget.cpp
    src/gui/settings_widget.cpp
    src/gui/package_card.cpp
    src/gui/package_details_dialog.cpp
)

# Header files
set(HEADERS
    src/utils/logger.h
    src/utils/types.h
    
    # Core
    src/core/alpm_wrapper.h
    src/core/aur_helper.h
    src/core/package_manager.h
    
    # GUI
    src/gui/mainwindow.h
    src/gui/home_widget.h
    src/gui/search_widget.h
    src/gui/installed_widget.h
    src/gui/updates_widget.h
    src/gui/settings_widget.h
    src/gui/package_card.h
    src/gui/package_details_dialog.h
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
	  resources.qrc
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    ${ALPM_LIBRARIES}
)

# Link directories
link_directories(${ALPM_LIBRARY_DIRS})

# Compiler flags
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
install(FILES assets/alg-app-store.desktop
    DESTINATION share/applications
)
