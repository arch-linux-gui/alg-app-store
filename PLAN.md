<!-- This file is intentionally untracked: not committed, not gitignored.
     It's a local planning reference only. -->

# Explorer Development Plan (0.3.x → 0.6.x)

This supersedes the ad-hoc items in `TODO.md` for anything covered below. `0.2.x`
was the Qt6/C++ rewrite baseline. Starting `0.3.x`, the project is renamed from
**ALG App Store** to **Explorer**, as part of ALG's project-wide push toward
single, memorable app names instead of `alg-<name>`.

**Repo:** renamed on GitHub → `https://github.com/arch-linux-gui/explorer.git`
(local folder and git remote already updated to match).

**Language baseline:** C++20 (confirmed — not 23 for now).

**Branch:** work for this release line happens on `0.3.x`.

**Note on numbering:** the `0.3.0`/`0.3.1`/... labels below are milestone
*ordering* labels for this plan, not literal targets for the actual
`VERSION` file. `VERSION`'s patch component auto-bumps on every commit via
the pre-commit hook (manually bumped to the `0.3.x` minor line, currently
`0.3.3`), so it will not line up 1:1 with these milestone numbers — treat
the milestone numbers as "do this work in roughly this order," not as a
version to hit.

---

## 0.3.x — Foundation & Rebrand

Housekeeping release line. No user-facing feature work; the goal is a clean,
testable, properly branded base for the security work in 0.4.x.

### 0.3.0 — Versioning infrastructure ✅ done
- [x] `VERSION` file at repo root as the single source of truth for the
      project version (starts at `0.2.30`, matching the last hardcoded
      CMake value)
- [x] `CMakeLists.txt` reads `PROJECT_VERSION_STRING` from `VERSION` via
      `file(STRINGS ...)` instead of hardcoding the version — verified with
      a real `cmake` configure (`CMAKE_PROJECT_VERSION` resolves to `0.2.30`)
- [x] `scripts/bump-version.sh` — pre-commit hook that auto-bumps the
      **patch** component of `VERSION` on every commit; minor/major stay a
      deliberate manual edit (hook no-ops if `VERSION` is already staged)
- [x] `scripts/install-hooks.sh` — one-time setup script, symlinks the hook
      into `.git/hooks/pre-commit`
- [x] Hook installed locally and confirmed working

### 0.3.1 — Rename `alg-app-store` → `Explorer`
- [x] Repo renamed on GitHub → `arch-linux-gui/explorer`
- [x] Local folder renamed to `explorer`
- [x] Local git remote (`origin`) updated to new URL
- [x] `CMakeLists.txt`: `project(explorer ...)`, binary target renamed
      (verified with a clean configure + build — binary is `explorer`,
      `CMAKE_PROJECT_VERSION` resolves to `0.3.3`); also added a missing
      `install(FILES assets/explorer.png DESTINATION share/pixmaps)` rule
      so the desktop file's `Icon=` actually resolves after install
- [x] `main.cpp`: `setApplicationName`, `setOrganizationName` strings updated
- [x] `mainwindow.cpp`/`mainwindow.h`: window title (`"ALG App Store (Beta)"`
      → `"Explorer (Beta)"`), About dialog text, doc comment
- [x] `assets/`: renamed `alg-app-store.desktop` → `explorer.desktop`
      (`Name=`, `GenericName=`, `Exec=`, `Icon=` updated); renamed
      `alg-app-store.png` → `explorer.png`
- [x] `README.md`, `CONTRIBUTING.md`, `build.sh`: updated all references,
      clone URLs, binary paths
- [x] `.github/workflows/release.yml`: artifact naming, job/release names,
      release body text updated; also fixed two pre-existing bugs found
      while touching this file — the version-extraction regex still
      grepped for a literal `project(alg-app-store VERSION x.y.z` in
      `CMakeLists.txt`, which broke when 0.3.0 switched to reading
      `PROJECT_VERSION_STRING` from the `VERSION` file (now `cat VERSION`
      directly); and the release-trigger check still watched
      `CMakeLists.txt` for changes instead of `VERSION` (now watches
      `VERSION`, and `VERSION` was added to the workflow's trigger `paths`
      so a patch-bump-only push isn't silently skipped)
- [x] `.gitignore`: `alg-app-store` → `explorer` ignored-binary entry
- [ ] **External/coordination task (not in this repo):** PKGBUILD lives in
      ALG's packaging repo — needs a coordinated rename PR there
      (`pkgname=explorer`, with `provides=`/`conflicts=`/`replaces=` against
      the old `alg-app-store` package so existing installs upgrade cleanly
      instead of ending up with both installed side by side)

### 0.3.2 — Build system modernization ✅ done
- [x] Bump `CMAKE_CXX_STANDARD` to `20` (verified: `-std=gnu++20` in the
      generated compile flags)
- [x] Split into granular CMake, one `CMakeLists.txt` per source folder:
  - `src/core/CMakeLists.txt` → builds `core` (static lib): alpm wrapper,
    AUR helper, package manager. Pure Qt Core/Network, no Widgets — kept
    that way deliberately so it stays a viable seam for the ALPM mock
    backend in 0.5.x. Publicly exposes ALPM include/link dirs so `gui`
    and the `explorer` target get them transitively.
  - `src/gui/CMakeLists.txt` → builds `gui` (static lib), links `core` +
    `utils`, owns `resources.qrc`
  - `src/utils/CMakeLists.txt` → header-only `INTERFACE` target (logger,
    types), links `Qt6::Core`, exposes `src/` as the include root
  - `tests/CMakeLists.txt` → still to come in 0.3.5, links `core` only (no
    Qt Widgets needed for pure-logic tests)
- [x] Root `CMakeLists.txt` shrunk to: version read, `project()`, C++
      standard + AUTOMOC/AUTORCC/AUTOUIC globals, `find_package` calls
      (Qt6, libalpm), `add_subdirectory()` calls, the thin `explorer`
      executable target (just `main.cpp`, linking `gui`/`core`/`utils`),
      install rules
- [x] Verified with a full clean configure + build (`explorer` binary
      links, runs libalpm at version 16.0.1 via `ldd`)
- [x] Fixed stale `C++17` mentions accompanying the bump: `README.md`,
      `CONTRIBUTING.md` (contribution guidelines + project structure tree,
      now showing the per-folder `CMakeLists.txt` files), and the About
      dialog text in `mainwindow.cpp`
- [x] **Regression found post-merge (caught by manual testing, not the
      build):** moving `resources.qrc` into the static `gui` library broke
      QSS loading at runtime — all four stylesheet modules failed silently
      (`"Could not load style module: ..."` warnings, app ran unstyled).
      Root cause: a static library's Qt resource initializer only gets
      linked into the final binary if something references it; nothing
      did. Fixed with `Q_INIT_RESOURCE(resources);` in `main.cpp` right
      after `QApplication` construction. Verified headless
      (`QT_QPA_PLATFORM=offscreen`) that stylesheets now load successfully.

### 0.3.3 — Logging overhaul ✅ done
- [x] Replaced `src/utils/logger.h` (qDebug wrapper) with spdlog
      (`find_package(spdlog REQUIRED)`, linked into `utils`); `utils` is now
      a real static library (`logging.cpp`/`logging.h`) instead of a
      header-only `INTERFACE` target, since it needs compiled CLI-parsing
      logic
- [x] `src/utils/version.h.in`'s generated header and the new logging setup
      both live behind `utils`, keeping `main.cpp`/`gui` decoupled from the
      details
- [x] `Log::init(argc, argv)` called at the top of `main()`, before
      `QApplication` construction: parses and strips `-v`/`-vv`(+)/`-D <N>`
      from argv so Qt never sees them
  - No flags: `debug` in dev builds, `info` in Release builds (`NDEBUG`)
  - `-v` → `debug`, `-vv` (or more `v`s) → `trace`
  - `-D <N>` → explicit `spdlog::level::level_enum` (0=trace..6=off),
    overrides `-v` when both given
  - Verified with actual dev + `-DCMAKE_BUILD_TYPE=Release` builds: default
    Release run shows only info+ lines, `-v` on that same Release binary
    correctly surfaces `debug` lines
- [x] Retired the `Logger::` namespace entirely; all 109 call sites across
      `core`+`gui`+`main.cpp` now call `spdlog::info/warn/error/debug`
      directly. Plain string-literal messages pass through unchanged;
      dynamic messages (anything built from a `QString`) are passed as a
      format *argument* — `spdlog::info("{}", msg.toStdString())` — never
      as the format string itself, so arbitrary content (pacman/AUR output,
      JSON, file contents) containing literal `{`/`}` can't be misparsed as
      a fmt placeholder and crash the logger
- [x] `CONTRIBUTING.md`: rewrote the "Logging" section and project
      structure tree to match (spdlog usage + the `-v`/`-D` flags,
      `logging.{h,cpp}` instead of `logger.h`)

### 0.3.4 — clang-format enforcement
- [ ] Finalize `.clang-format` ruleset
- [ ] One isolated repo-wide reformat commit (formatting only, no logic
      changes, easy to review/skip in blame)
- [ ] CI job: `clang-format --dry-run --Werror` across `src/`, gating PRs

### 0.3.5 — Test infrastructure
- [ ] Catch2 as the test framework, driven through CTest
      (`add_test()` + `ctest` in CI — Catch2 registers tests, CTest runs them)
- [ ] **Scope for this release: pure logic only.** `AlpmWrapper` and
      `PackageManager` remain hard singletons touching real `libalpm` /
      `pkexec` / `/etc/pacman.conf` — not mocked yet. Extract and cover what's
      already (or easily made) pure:
  - AUR JSON → `PackageInfo` parsing (`AurHelper::parseAurPackage`)
  - `pacman.conf` section parsing (repo detection, multilib/chaotic-aur
    enabled checks) — extract into free functions taking a `QString`/stream
    so tests can feed fixture text instead of hitting the real file
  - Update-diff / version-compare call sites
  - Progress-output regex parsing (`PackageDetailsDialog::parseProgressOutput`)
- [ ] Full ALPM mock backend (so `AlpmWrapper`'s own logic is testable) is
      **explicitly deferred to 0.5.x**, alongside the architecture cleanup
      that gives it a real seam to mock against
- [ ] CI job running `ctest`

### 0.3.6 — std::jthread / RAII pass
- [ ] Not a blanket rewrite — the codebase has no raw `std::thread` today
      (it's `QtConcurrent::run` + Qt signals throughout). Target the two
      spots that actually need cooperative cancellation:
  - `PackageManager::cancelRunningOperation()` — currently a manual
    `pkill`/`terminate`/`waitForFinished` sequence; candidate for
    `jthread` + `stop_token`
  - AUR update-polling loop (`AurHelper::checkAurUpdates`) — also touched
    in 0.5.x when it's batched into a single RPC call, so sequence these
    together

---

## 0.4.x — Security & Trust hardening

Everything here is either a real vulnerability in the current code or an
Arch-specific correctness risk. Concentrated in one release line per your
call, ahead of the architecture/feature work.

### 0.4.0 — Fix `pkexec` command injection
- [ ] `PackageManager::installPackage/updatePackage/updateAllPackages`
      currently build `sh -c "pkexec %1 -S %2 --noconfirm"` via
      `QString::arg()` with package names sourced from AUR JSON / search
      results — a shell-metacharacter-laced name is exploitable
- [ ] Replace with direct argv `QProcess::start("pkexec", {"pacman", "-S",
      packageName, "--noconfirm"})` (no `sh -c` wrapper) everywhere

### 0.4.1 — Fix pacman.conf tmpfile TOCTOU
- [ ] `SettingsWidget`'s multilib/chaotic-aur toggles write to a fixed,
      predictable `/tmp/pacman.conf.tmp` before `pkexec cp` — symlink/race
      risk. Replace with `QTemporaryFile`
- [ ] Move all `pacman.conf` parse/rewrite logic out of `SettingsWidget` and
      into a new core `RepoConfigManager` (also unblocks proper unit testing
      of this logic per 0.3.5/0.5.x)

### 0.4.2 — Safer uninstall default
- [ ] Default single-package uninstall: `pacman -Rdd` → `pacman -Rs`
      (dependency-aware removal)
- [ ] Keep `-Rdd` (skip dependency checks) available as an explicit,
      clearly-labeled "force remove" advanced action, not the default path

### 0.4.3 — Partial-upgrade guardrail
- [ ] Detect when the local package DB is stale relative to sync DBs before
      allowing a single-package install/update
- [ ] Warn the user about partial-upgrade risk (a well-known way to break an
      Arch system) and steer toward a full `-Syu` when appropriate

### 0.4.4 — PKGBUILD visibility for AUR packages
- [ ] Fetch and display PKGBUILD / `.SRCINFO` in the package details dialog
      before an AUR install, so users can review the build script they're
      about to run

### 0.4.5 — Real transaction preview (stretch)
- [ ] Use `alpm_trans_*` to compute and show actual dependency/conflict
      resolution before confirming an install, instead of trusting raw
      `pacman -S` output after the fact

---

## 0.5.x — Architecture cleanup

- [ ] Centralize ALPM release+reinitialize-after-mutation (currently
      duplicated across `UpdatesWidget` and `PackageDetailsDialog`, both
      success and error paths — 4 copies)
- [ ] Dedupe `formatSize()` (duplicated in `UpdatesWidget` and its nested
      `UpdateItem`)
- [ ] Extract a shared `PackageGridView` widget — the row/col grid-packing
      loop is triplicated across `HomeWidget`, `SearchWidget`,
      `InstalledWidget`
- [ ] `std::optional<PackageInfo>` instead of returning an empty struct as a
      failure sentinel
- [ ] Repository enum/normalization instead of scattered
      `"aur"`/`"AUR"`/`"chaotic-aur"` string comparisons with inconsistent
      casing
- [ ] Remove dead `PackageManager::Helper::Paru` code (currently commented
      out and unreachable) or properly re-enable it
- [ ] Replace nested blocking `QEventLoop` calls (`HomeWidget`/`SearchWidget`
      package-click → AUR detail fetch) with proper async signal
      continuations
- [ ] Batch AUR update-checking into a single multi-`arg[]` RPC call instead
      of N sequential blocking calls (pairs with the 0.3.6 jthread work)
- [ ] Introduce the full ALPM mock-backend seam deferred from 0.3.5, backfill
      unit test coverage for `AlpmWrapper`/`PackageManager` logic

---

## 0.6.x — Feature round

- [ ] Single sudo prompt at startup (`startup_auth`, carried over from the
      old `TODO.md`)
- [ ] Own lightweight AUR helper in core, reducing the hard dependency on
      yay/paru
- [ ] Mirrorlist management tab
- [ ] Light/dark theme toggle, or follow system theme
- [ ] Settings page UX pass
- [ ] Package categories/tags
- [ ] Transaction history / persistent operation log
- [ ] Dependency visualization

---

## Decisions log

| # | Question | Decision |
|---|----------|----------|
| 1 | C++20 vs C++23 baseline | **C++20** |
| 2 | Rename scope | **Full**, including the GitHub repo itself — done, new URL is `https://github.com/arch-linux-gui/explorer.git` |
| 3 | 0.3.5 test scope | **Pure logic gets unit tests now; full ALPM mock backend lands in 0.5.x** |
| 4 | Versioning hook semantics | Confirmed: `bump-version.sh` bumps the **patch** component only, minor/major stay manual |
