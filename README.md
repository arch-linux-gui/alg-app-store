# ALG App Store - Qt6/C++ Version

A modern, native package manager for Arch Linux built with Qt6 and C++17. This is a complete rewrite of the original Wails-based application.

## Features

- **Search Packages**: Search through official Arch repositories (core, extra) and AUR
- **View Installed Packages**: Browse and manage installed packages
- **Check for Updates**: View available updates for both official and AUR packages
- **Package Management**: Install, uninstall, and update packages
- **Modern UI**: Clean, dark-themed interface with responsive design
- **Smart Helper Detection**: Automatically detects and uses yay, paru, or falls back to pacman
- **Thread-Safe**: Uses modern C++ threading features for safe concurrent operations
- **Comprehensive Logging**: Built-in logger for debugging and monitoring

## Technology Stack

- **Language**: C++17
- **GUI Framework**: Qt6 (Widgets)
- **Package Management**: libalpm (Arch Linux Package Manager library)
- **AUR Integration**: AUR RPC API via Qt Network
- **Build System**: CMake
- **Threading**: Qt Concurrent & STL threading

## Prerequisites

### Build Dependencies

```bash
sudo pacman -S base-devel cmake qt6-base qt6-svg alpm pkgconf
```

### Runtime Dependencies

```bash
sudo pacman -S qt6-base libalpm polkit curl
# Optional but recommended:
sudo pacman -S yay  # or paru
```

## Building

1. Clone the repository:
```bash
git clone https://github.com/arch-linux-gui/alg-app-store.git
cd alg-app-store
```

2. Run Build Script
```bash
# This will create a build directory.
./build.sh
```

Binary will be in the build directory.

## Running

### From Build Directory

```bash
./build/alg-app-store
```

### From System Installation (if installed)

```bash
alg-app-store
```


### Key Components

#### AlpmWrapper (Singleton)
Wraps libalpm functionality with thread-safe operations:
- Package searching
- Installed package enumeration
- Update detection
- Package information retrieval

#### AurHelper
Handles AUR integration:
- Package search via AUR RPC API
- Package information retrieval
- Update checking for AUR packages

#### PackageManager (Singleton)
Manages package operations with proper privilege escalation:
- Install/uninstall packages
- Update operations
- Automatic helper detection (yay/paru/pacman)
- Process management with signals

#### GUI Components
- **MainWindow**: Tabbed interface container
- **HomeWidget**: Featured packages display
- **SearchWidget**: Package search with filtering
- **InstalledWidget**: Installed package browser
- **UpdatesWidget**: Update management
- **PackageCard**: Reusable package display widget
- **PackageDetailsDialog**: Detailed package information

## Design Decisions

### Modern C++ Features

- **Smart Pointers**: `std::unique_ptr` and `std::shared_ptr` for automatic memory management
- **Move Semantics**: `std::move()` for efficient resource transfer
- **Auto Type Deduction**: Cleaner, more maintainable code
- **Range-based Loops**: Cleaner iteration
- **Lambda Functions**: Inline callbacks and signal connections

### Thread Safety

- **Mutexes**: `std::mutex` and `std::lock_guard` for critical sections
- **Qt Concurrent**: `QtConcurrent::run()` for background operations
- **Signal/Slot**: Qt's thread-safe communication mechanism

### Singleton Pattern

Used for `AlpmWrapper` and `PackageManager` to ensure:
- Single libalpm handle instance
- Centralized package operation management
- Thread-safe access

## Logging

The application includes a comprehensive logging system:

```cpp
Logger::info("Information message");
Logger::warning("Warning message");
Logger::error("Error message");
Logger::debug("Debug message");
```

Logs are output to standard output and can be redirected for persistent logging.

## Package Helper Detection

The application automatically detects available package helpers in this order:

1. **yay** - Preferred for AUR support
2. **paru** - Alternative AUR helper
3. **pacman** - Fallback (official repos only)

## Privilege Escalation

Package operations require root privileges. The application uses `pkexec` (PolicyKit) for secure privilege escalation. Ensure PolicyKit is properly configured on your system.

## Contributing

Contributions are welcome! Please ensure:

- Code follows C++17 standards
- Proper error handling and logging
- Thread safety for concurrent operations
- Qt best practices for GUI code
- Comments for complex logic

## License

This project is part of the Arch Linux GUI project.

## Credits

- **Original Project**: Wails-based ALG App Store
- **Rewrite**: Qt6/C++ implementation
- **Community**: Arch Linux and Qt communities

## Troubleshooting

### Build Issues

**Missing Qt6**:
```bash
sudo pacman -S qt6-base qt6-svg
```

**Missing libalpm**:
```bash
sudo pacman -S pacman
```

### Runtime Issues

**Permission Denied**:
Ensure PolicyKit is running:
```bash
systemctl status polkit
```

**Package Helper Not Found**:
Install yay or paru:
```bash
sudo pacman -S yay
```

**ALPM Initialization Failed**:
Check pacman configuration:
```bash
sudo pacman -Syy
```

## Future Enhancements

- Package categories/tags
- Package ratings and reviews
- Automated testing suite
- Flatpak integration
- AppImage support
- Configuration file support
- Multi-language support
- Package download progress
- Transaction history
- Dependency visualization

## Contact

For issues, questions, or contributions, please visit:
- GitHub: https://github.com/arch-linux-gui/alg-app-store
- Website: https://archlinuxgui.in
