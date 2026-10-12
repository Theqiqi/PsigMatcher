**English** | [简体中文](README.zh-CN.md)

# PsigMatcher — batch signature matcher for Windows processes

A desktop tool that maintains signatures in a **table** and **batch-scans Windows process memory** against them: the input is a signature table, the output is a match-result table, and both can be viewed, edited, imported and exported inside the app.

- Repository: <https://github.com/Theqiqi/PsigMatcher>
- Version: **v0.3.3** (tag `v0.3.3`; code and design documents are archived under the same number)
- Shape: Windows desktop Qt6 GUI monolith (`PsigMatcher.exe`) + business static library (`psig.lib`)
- Language / toolchain: C++20 / MSVC (`cl.exe`) + Ninja + CMake ≥ 3.20 + vcpkg manifest
- License: MIT (see [License](#license))

## Highlights

**Input side (signature table)**

- Table-based editing; columns can be freely added/removed (built-in `name / signature / comment` plus the GUI-managed `process` column).
- Signature patterns support wildcards: `48 8B ?? 05`, `488B??05`, `?8 8B A?` and similar forms.
- Import: signature table (CSV / SQLite `.db`); export: signature table (CSV / `.db`; the `.db` carries the target process name).
- **Real dirty tracking**: cell edits, row insert/delete and column changes all count as unsaved; a successful import/export clears the flag; the persistent status area shows a `*`.
- With unsaved changes: import asks for **overwrite confirmation only** (Continue / Cancel); closing the app or "re-select process" prompts **Save / Don't save / Cancel**.

**Output side (match-result table)**

- Batch scan of many processes × many signatures; results are summarised per row (signature name, process name/PID, address, module, matched bytes, match time, custom columns).
- The result page supports row deletion, editing the trailing custom column, batch fill, choosing which columns to export, and a table-level template text.
- Export: CSV / TXT (GUI entry points); the static library additionally supports result `.db`.

**Other**

- Chinese / English UI switching (export headers and the TXT summary follow the language); process list filtering, sorting and access-rights icons.
- Scan progress bar, a history-log dock (coloured by level), and a consistent white-plus-accent visual style.
- **Embedded application icon**: `PsigMatcher.exe` carries a multi-size icon resource, and the window/taskbar/Alt-Tab icon uses native multi-size frames.

## Repository layout

```
env/                        project root (only source subdirectories and config, no build artifacts)
├─ gui/                     the GUI monolith (MVVM, 4 layers)
│  ├─ CMakeLists.txt        project(PsigMatcher): shared options + subdirectory entries
│  ├─ CMakePresets.json     debug (shared Qt /MDd) and release (static Qt /MT) presets
│  ├─ vcpkg.json            gtest (Qt6 is installed standalone, not via vcpkg)
│  ├─ src/
│  │  ├─ main.cpp           assembly: create View → create ViewModel → install(view) → show → exec
│  │  ├─ view/              UI only (no signals / no connect / no event logic)
│  │  ├─ model/             pure data (QAbstractTableModel, no business logic)
│  │  └─ viewmodel/         ViewModel (holds View*/Model*, central connect, calls the static library)
│  ├─ res/                  resources.qrc, style.qss, app.ico / app.rc / app_<N>.png, i18n/*.ts
│  ├─ script/               build / test / verify / one-off scripts (see script/README.md)
│  ├─ test/                 unit | integration | target | e2e
│  └─ out/                  build output (out/bin/<config>/PsigMatcher.exe, out/lib, out/build; gitignored)
├─ psig/                    business static library (single external contract: include/psig.h)
│  ├─ include/psig.h        public API (no Qt dependency, no exceptions, thread-safe)
│  ├─ src/data → algorithm → api   bottom-up layering; src/atomic = Win32 / file / SQLite primitives
│  ├─ third_party/sqlite3/  bundled SQLite (for `.db` read/write)
│  ├─ test/                 unit (psig_unit) | integration (psig_integration) | target (psig_target)
│  ├─ script/               build_debug/release (.bat + _wsl.sh)
│  └─ out/lib/<config>/psig.lib
├─ design/                  design documents archived per version (v0.1.0, v0.2.x, v0.3.x, v0.3.3 = current)
├─ .flow/                   process specifications (new program / modify & add / static library)
└─ .idea/                   per-version requirement input (idea.md, v0.3.3.md, …)
```

## Requirements

| Item | Requirement |
| --- | --- |
| OS | Windows 10/11 x64 (a real desktop is required; the GUI uses Qt Widgets and Win32 process-memory access) |
| Toolchain | Visual Studio 2022 (MSVC x64) + Ninja + CMake ≥ 3.20 |
| Resource compiler | `rc.exe` from the Windows SDK (provided by `vcvars64.bat`); used to embed the exe icon resource |
| Package manager | vcpkg (defaults to the one bundled with VS2022; override with `VCPKG_ROOT`) — used for gtest only |
| Qt | Qt 6.10.2, **installed standalone** (not in the vcpkg manifest): shared Debug build, static Release build |
| Optional | WSL (`cmd.exe` as the entry point for the `.bat` scripts) |
| Optional (icon tooling) | Python 3 + Pillow, PowerShell — development-time only, never a build/runtime dependency |

Environment variables (the scripts contain fallback defaults; override them when moving machines):

```bat
set VCPKG_ROOT=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg
set QT6_DEBUG_DIR=C:/d/ccpplib/qt6.10.2-windows-x86_64-msvc2022-shared_debug
set QT6_RELEASE_DIR=C:/d/ccpplib/Qt_6.10.2-static-Release_msvc2022_64/msvc2022_64
```

## Build & run

**The build order is fixed: static library first, then the GUI** (the GUI references `psig/out/lib/<config>/psig.lib` through `find_library`).

```bash
# 1) static library (Release: static /MT; Debug: shared /MDd)
bash psig/script/build_release_wsl.sh      # or bash psig/script/build_debug_wsl.sh
#    → psig/out/lib/Release/psig.lib

# 2) GUI (must use the same configuration as the library)
bash gui/script/build_release_wsl.sh       # or bash gui/script/build_debug_wsl.sh
#    → gui/out/bin/Release/PsigMatcher.exe

# 3) run (Release uses static Qt: self-contained, no console window)
gui/out/bin/Release/PsigMatcher.exe
```

- Windows-side equivalents: run `psig\script\build_release.bat` and `gui\script\build_release.bat` directly.
- Debug uses **shared Qt**: the Qt `bin` directory must be on `PATH` at runtime (`run_*_tests*.bat` handles this); Debug keeps a console for debugging, Release does not.
- Presets: `debug` (`x64-windows`, `/MDd`) and `release` (`x64-windows-static`, `/MT`) in `gui/CMakePresets.json` / `psig/CMakePresets.json`.

## Tests

| Level | Location | How to run |
| --- | --- | --- |
| Static library unit + integration | `psig/test/unit`, `psig/test/integration` | `cd psig/out/build/release && ctest --output-on-failure` (2 test targets) |
| GUI unit (View / Model / ViewModel) | `gui/test/unit` | `bash gui/script/run_release_tests_wsl.sh` (Debug: `run_debug_tests_wsl.sh`) |
| GUI integration + user flows | `gui/test/integration` | same command (ctest runs them together; 5 test targets in total) |
| E2E (real launch + input injection + pixel and artifact assertions) | `gui/test/e2e` | `bash gui/test/e2e/verify_all.sh Release` |

- GUI tests run with `QT_QPA_PLATFORM=offscreen`, so they work on machines without a display.
- The E2E order is fixed: kill leftover processes → launch the app → inject real keystrokes / `WM_CLOSE` → capture the client area → pixel assertions (`analyze.py`) → icon-resource verification (`check_app_icon.py`, `extract_exe_icon.ps1`, `check_exe_icon.py`). No administrator rights are needed. The case list lives in `gui/test/e2e/cases.md`.

## Application icon

`PsigMatcher.exe` embeds a multi-size icon resource (PE `RT_GROUP_ICON` + 7 `RT_ICON` entries) and the window icon is assembled from the same seven sizes, so the title bar, taskbar, Alt-Tab and Explorer all get a native frame.

| Item | Detail |
| --- | --- |
| Sizes | 16 / 24 / 32 / 48 / 64 / 128 / 256 (128 and 256 are stored as PNG entries inside the `.ico`) |
| Visual identity | dark blue background `#2A4A7A` with a light blue diagonal band `#6FA8E0`; a pixel is part of the band when `|u−v| ≤ 5/32` (normalised pixel-centre coordinates) |
| Artifacts | `gui/res/app.ico` (exe resource, referenced by `gui/res/app.rc`), `gui/res/app_<N>.png` (7 files in `resources.qrc`, loaded by `main.cpp` via `QIcon::addFile`) |
| Generator | `gui/script/gen_app_icon.py` (Python 3 + Pillow, development-time only) |

```bash
# regenerate every icon artifact (the .ico and the PNG set are derived files)
python3 gui/script/gen_app_icon.py

# source-side consistency: artifacts reproducible from the script, frames == PNGs,
# and the size set identical in gen_app_icon.py / resources.qrc / main.cpp
python3 gui/script/verify/check_app_icon.py

# artifact side: PE resource section of the built exe (types, sizes, byte-identical frames)
python3 gui/script/verify/check_exe_icon.py gui/out/bin/Release/PsigMatcher.exe

# independent evidence: the icon Windows itself associates with the file (Explorer's logic)
powershell -NoProfile -ExecutionPolicy Bypass -File gui/script/verify/extract_exe_icon.ps1 \
  -Exe gui/out/bin/Release/PsigMatcher.exe -OutDir gui/out/verify
python3 gui/script/verify/check_exe_icon.py gui/out/bin/Release/PsigMatcher.exe \
  --shell-icon gui/out/verify/exe_associated_icon.png
```

- Never hand-edit the generated binaries: the only writer is `gen_app_icon.py`, which keeps regeneration byte-for-byte deterministic.
- The 32×32 frame is pixel-identical to the single-size `app.png` shipped before v0.3.3 (that file was superseded by `app_32.png`).
- Pillow is intentionally **not** added to `gui/vcpkg.json`: icon tooling never enters the product dependency tree.

## Static library API (`psig/include/psig.h`)

The single external contract; the GUI may only call through it. The library does not depend on Qt and **does not throw** (errors are reported through return values / out parameters).

- Processes and scanning: `EnumerateProcesses`, `ProcessIcon`, `DefaultProcessIcon`, `ParsePattern`, `ScanProcesses` (parallel over memory regions, accepts a progress callback and a cancel flag), `RequestCancel`
- Signature tables: `BuiltinSignatureColumns`, `ImportSignaturesCsv/Db`, `ExportSignaturesCsv/Db`
- Match results: `ExportResultsCsv/Db/Txt`, `ResultColumnDefs`, `SelectResultKeys`
- Structs: `ProcessInfo`, `Pattern`, `MatchResult`, `Table`/`TableRow`/`ColumnDef`, `ResultExportOptions`, `ScanRequest`/`ScanProgress`

Conventions: CSV read/write handles a UTF-8 BOM; `.db` means SQLite (extension `.db`); the scan progress callback may be invoked concurrently from several worker threads, so implementations must be thread-safe and must never touch the UI directly.

## Design and process documents

- `design/v<version>/gui/`: GUI-side analysis → design → interfaces → review matrices → E2E results (numbered 00–35, e.g. `04_GUI需求分析.md`, `24_ViewModel接口.md`, `31_ViewModel层审查矩阵.md`).
- `design/v<version>/gproto/`: static-library-side documents; `design/v<version>/gui/00_CHANGELOG.md` records "this version = previous version + list of changes".
- **Archived version directories are never edited in place**: every change creates a new `design/v<version>/`, and the version number corresponds one-to-one with a git tag.
- `.flow/` holds the process specifications: use `带GUI的单体程序的GUI入口系统.md` + `带GUI的单体程序的业务系统静态库.md` for a new program, and `带GUI的单体程序的添加与修改.md` for incremental changes, with the requirement input in `.idea/v<version>.md`.

## Project conventions

- **Layer boundaries**: the View only draws UI (no `connect`, no signals, no event logic; it compiles standalone); the Model stays pure (no business or formatting logic); the ViewModel owns the `connect` calls, reads widgets through public View getters and refreshes the UI through public View methods (never `findChild`); every business capability comes from the static-library public API — the GUI never re-implements or fabricates data.
- **Naming**: production exe = `PsigMatcher` (renamed from `psig_gui` in v0.3.2); static library = `psig`; test targets = `psig_test_*` / `psig_target`.
- **Script management**: `gui/script/` is organised into four categories (core / verify / diagnose / one-off); new scripts must be classified and `gui/script/README.md` updated accordingly. Development-time Python scripts must fail with a non-zero exit code instead of silently degrading.
- **Commit messages**: `type: content` (chore/docs/feat/fix/test), content in English.
- **Language**: UI strings go through `tr()`, with Chinese and English translations in `gui/res/i18n/PsigMatcher_{zh_CN,en_US}.ts` (Chinese is the source language). Remember to update both `<source>` and `<translation>` when a version number appears in UI text.
- **Icon assets**: generated only by `gui/script/gen_app_icon.py`; adding or removing a size means updating the generator, `resources.qrc` and `main.cpp` together.

## Version notes

| Version | Highlights |
| --- | --- |
| v0.1.0 → v0.2.x | first complete version plus increments: language switching, process icons/column widths, log dock, scan progress and parallelism, result column selection, etc. |
| v0.3.0 | scan progress shows real progress, region-level parallelism, timer-sampled refresh |
| v0.3.1 | process-table hover/selection painting fixed, corner select-all disabled, signature `.db` carries the target process name |
| v0.3.2 | real unsaved-change detection (including cell edits) + persistent `*` marker; import decoupled from save (overwrite confirmation only); close/quit prompt for unsaved changes (menu "Quit" actually works); program renamed to **`PsigMatcher.exe`** |
| **v0.3.3 (current)** | **application icon resources** for the compiled binary (multi-size PE icon resource + multi-size window icon, reproducibly generated); README refreshed; **bilingual README (this English file plus [`README.zh-CN.md`](README.zh-CN.md))** |

## FAQ

| Symptom | Handling |
| --- | --- |
| Configure fails with "psig.lib not found" | Build the static library first (`bash psig/script/build_release_wsl.sh`) and make sure the GUI and library configurations match |
| Debug build fails to start with missing `Qt6*.dll` | Debug uses shared Qt; add `%QT6_DEBUG_DIR%/bin` to `PATH` (or use `run_debug_tests*.bat`) |
| Release build prints nothing to the console | Expected: Release is a static-Qt program using the `WIN32` subsystem; use the Debug build when diagnosing |
| A scan returns nothing for some process / is denied | Reading another process's memory requires sufficient privileges; the process table shows accessibility through an icon |
| E2E reports that the build artifact is missing | Run the E2E against the `Release` artifact: `bash gui/script/build_release_wsl.sh` first |
| The exe shows a generic icon in Explorer | Verify with `python3 gui/script/verify/check_exe_icon.py <exe>`; if the PE resource is missing, rebuild (the `.rc` compile step is part of the build log) |

## License

This project is released under the **MIT** license; see [LICENSE](LICENSE).

```
MIT License

Copyright (c) 2026 Theqiqi
```
