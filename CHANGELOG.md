# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2026-05-16

### Added
- **JSON Input Support**: Implemented a file selection dialog for JSON input plugins, similar to the CSV functionality.
- **Configurable Upload Payloads**: 
  - Topics can now override upload payload options via the INI file (`[topics.<Topic>.options]`).
  - Added support for topic-specific `dateFormat` presets in the configuration.
- **Flexible Date Formatting UI**:
  - Added new GUI controls to select date component order and delimiters.
  - Automatic synchronization of date format settings when switching topics.
  - Dynamic merging of GUI settings and INI overrides during the upload process.
- **Modernized Update Checker**: 
  - Upgraded `gh-update-checker` to v1.1.0.
  - Implemented asynchronous update checks to keep the UI responsive.
  - Added URL sanitization to handle `www.` and protocol variations for more robust GitHub API calls.

### Changed
- **Build System**: Cleaned up `CMakeUserPresets.json` to avoid duplicate preset errors from nested build directories.
- **Sample Configuration**: Exhaustively updated `sample.ini` with documentation and examples for all new features.

## [0.2.0] - 2026-05-15

### Added
- **Dual-Plugin Strategy**: Fully implemented split between Data (Ingest) and Upload (Output) plugins.
- **Dynamic Interface Discovery**: The application now scans for plugins following the `<topic>_<type>_input` convention.
- **Dynamic GUI Dropdown**: Replaced the static "Load CSV" button with a topic-aware interface selection menu.
- **Plugin Management**: Added a new "Manage -> Plugins" dialog to list all discovered plugins, their topics, types, and versions.
- **GitHub Update Checker**: Integrated `gh-update-checker` to notify users of new releases in the "About" dialog.
- **Enhanced Logging**:
  - Implemented a GUI log sink to display `spdlog` messages directly in the "Log/Status" area.
  - Added OS user identification and session lifecycle logging (start/end) to the Core log.
  - Improved audit trail by logging the initiating OS user during upload operations in the topic log.
- **Configuration Pre-population**: INI configuration now supports `data_<interface>` keys to suggest default paths or connection strings in the GUI.
- **GUI Restriction**: Added `only_default_datasource` configuration to lock the UI to pre-configured data sources.

### Fixed
- **Stability**: Fixed a critical crash caused by multiple `dlerror()` calls on Linux.
- **Stability**: Wrapped initialization in `try-catch` blocks to prevent crashes on malformed configuration.
- **Parsing**: Updated the INI handler to automatically strip double quotes from configuration values.
- **Concurrency**: Ensured all UI updates from background threads are performed safely via `wxWindow::CallAfter`.

## [0.1.0] - 2025-02-13

### Added
- Initial project structure with `cgui_core` and `wxWidgets` front-end.
- Secure configuration decryption via `libsodium`.
- Basic CSV, DB, and Kafka plugin stubs.
- JSON Schema validation and asynchronous upload tasks.
