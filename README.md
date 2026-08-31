# Explorer - Qt6/C++ Version

A modern, native package manager for Arch Linux built with Qt6 and C++20. This is a complete rewrite of the original Wails-based application (formerly known as ALG App Store).

## Features

- **Search Packages**: Search through official Arch repositories (core, extra) and AUR
- **View Installed Packages**: Browse and manage installed packages
- **Check for Updates**: View available updates for both official and AUR packages
- **Package Management**: Install, uninstall, and update packages
- **Modern UI**: Clean, dark-themed interface with responsive design
- **Smart Helper Detection**: Automatically detects and uses yay, paru, or falls back to pacman
- **Thread-Safe**: Uses modern C++ threading features for safe concurrent operations
- **Comprehensive Logging**: [spdlog](https://github.com/gabime/spdlog)-backed logging with configurable verbosity (see [Logging](#logging) below)

## Technology Stack

- **Language**: C++20
- **GUI Framework**: Qt6 (Widgets)
- **Package Management**: libalpm (Arch Linux Package Manager library)
- **AUR Integration**: AUR RPC API via Qt Network + Chaotic AUR Support
- **Build System**: CMake
- **Threading**: Qt Concurrent & STL threading

## Prerequisites

### Build Dependencies

```bash
sudo pacman -S base-devel cmake qt6-base qt6-svg alpm pkgconf spdlog catch2
```

You can optionally also have either either yay or paru if you would like to work with packages from the AUR.

`catch2` is only needed to build the test suite (see [Testing](#testing) below); pass `-DBUILD_TESTING=OFF` to `cmake` to skip it.

## Building

1. Clone the repository:
```bash
git clone https://github.com/arch-linux-gui/explorer.git
cd explorer
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
./build/explorer
```

### From System Installation (if installed)

```bash
explorer
```

## Logging

Explorer logs via [spdlog](https://github.com/gabime/spdlog) to standard
output. By default it logs at `debug` level in a development build and
`info` level in a Release build (`-DCMAKE_BUILD_TYPE=Release`).

Verbosity can be raised with `-v` flags or set explicitly with `-D <N>`:

| Flag       | Level        |
|------------|--------------|
| *(none)*   | `debug` (dev build) / `info` (Release build) |
| `-v`       | `debug`      |
| `-vv`      | `trace`      |
| `-D <N>`   | explicit level by number — `0`=trace, `1`=debug, `2`=info, `3`=warn, `4`=err, `5`=critical, `6`=off |

`-D <N>` takes precedence over `-v` if both are given. Examples:

```bash
# Default verbosity
./build/explorer

# Debug-level logging
./build/explorer -v

# Most verbose (trace)
./build/explorer -vv

# Explicit level: warnings and above only
./build/explorer -D 3
```

## Testing

Explorer has a [Catch2](https://github.com/catchorg/Catch2)-based unit test
suite, run through CTest. Coverage is currently pure logic only — AUR JSON
parsing, `pacman.conf` section parsing, and pacman/yay/paru progress-output
parsing. `AlpmWrapper`/`PackageManager` still talk to real `libalpm`/`pkexec`
and aren't covered yet.

```bash
cd build
ctest --output-on-failure
```

## License

This project is part of the Arch Linux GUI project.
It is distributed under the MIT License. Check LICENSE.

## Credits

- **Author**: DemonKiller
- **Original Project**: Wails-based ALG App Store (predecessor of Explorer)
- **Rewrite**: Qt6/C++ implementation
- **Community**: Arch Linux and Qt communities

## Contact

For issues, questions, or contributions, please visit:
- GitHub: https://github.com/arch-linux-gui/explorer
- Website: https://arkalinuxgui.org
- Discord: https://discord.com/invite/NgAFEw9Tkf