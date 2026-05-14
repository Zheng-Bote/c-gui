# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-02-13

### Added
- Implemented `MainWindow` class using wxWidgets.
- Implemented `AppController` class for application orchestration.
- Updated `main.cpp` to initialize wxWidgets app and handle configuration decryption.
- Added GUI components: Menu bar, Topic selection, CSV loading, Data preview, Validation, Upload, and Progress tracking.
- Integrated background threading for validation and upload tasks.
- Implemented Plugin Build System with RPATH support.
- Implemented CSV Input Plugin for reading and converting CSV data to JSON.
- Implemented DB Input Plugin (PostgreSQL) using libpqxx.
- Implemented Kafka Input Plugin (Stub) for future Kafka integration.
