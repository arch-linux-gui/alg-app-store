#pragma once

#include <spdlog/spdlog.h>

namespace Log {

// Parses and strips verbosity flags from argv, then configures the default
// spdlog logger's level accordingly. Must be called before constructing
// QApplication, so Qt never sees flags it doesn't recognize.
//
// With no flags: `debug` in dev builds, `info` in Release builds (NDEBUG).
// -v: `debug`. -vv (or more v's): `trace`.
// -D <N>: explicit spdlog::level::level_enum value (0=trace .. 6=off);
// takes precedence over -v when both are given.
void init(int &argc, char **argv);

} // namespace Log
