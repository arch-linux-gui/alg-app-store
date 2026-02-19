cmake_minimum_required(VERSION 3.16)

project(alg-app-store VERSION 0.3.0 LANGUAGES CXX)

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
    DBus
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
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::DBus
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

# Daemon executable
add_executable(alg-app-store-daemon
    src/daemon/main.cpp
    src/daemon/appstore_daemon.cpp
    src/daemon/appstore_daemon.h
)

target_link_libraries(alg-app-store-daemon PRIVATE
    Qt6::Core
    Qt6::DBus
)

target_compile_options(alg-app-store-daemon PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(TARGETS alg-app-store-daemon
    RUNTIME DESTINATION bin
)

# Install D-Bus config
install(FILES assets/org.arkalinuxgui.AppStore.conf
    DESTINATION share/dbus-1/system.d
)

# Install D-Bus service file
install(FILES assets/org.arkalinuxgui.AppStore.service
    DESTINATION share/dbus-1/system-services
)

# Install Polkit policy
install(FILES assets/org.arkalinuxgui.appstore.policy
    DESTINATION share/polkit-1/actions
)

# Install desktop file
install(FILES assets/alg-app-store.desktop
    DESTINATION share/applications
)

# Copy stylesheet to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/stylesheet.qss
    ${CMAKE_BINARY_DIR}/stylesheet.qss
    COPYONLY
)

# Install stylesheet
install(FILES stylesheet.qss
    DESTINATION share/alg-app-store
)
