# c-gui Technical Documentation

> Comprehensive technical documentation for the c-gui application.  
> **Version:** 0.5.0 | **Language:** C++23 | **License:** Apache-2.0

---

## Document Index

| # | Document | Description | Diagrams |
|---|----------|-------------|----------|
| 01 | [System Overview](01-overview.md) | Project introduction, technology stack, high-level architecture, directory layout | C4 Context (System) |
| 02 | [C4 Container & Component](02-architecture-c4-containers.md) | Container diagram, cgui_core component breakdown, GUI layout, threading model | C4 Container, C4 Component, Sequence, Block diagram |
| 03 | [UML Class Design](03-class-design.md) | Plugin interface hierarchy, core library classes, LogManager, MainWindow | UML Class diagrams |
| 04 | [Plugin System](04-plugin-system.md) | Dual-plugin architecture, interfaces, lifecycle, discovery, plugin inventory | State diagram, Sequence diagram |
| 05 | [Security Architecture](05-security-architecture.md) | Config encryption (XChaCha20-Poly1305 + Argon2id), Ed25519 audit signing, SaaS auth | Sequence diagrams, Flowchart |
| 06 | [Data Flow](06-data-flow.md) | End-to-end data pipeline: ingestion, validation, upload, payload merging | Flowchart, Sequence diagrams |
| 07 | [Build System](07-build-system.md) | CMake structure, Conan recipe, presets, CI/CD, output layout | Dependency graph |
| 08 | [Plugin Development Guide](08-plugin-development-guide.md) | Step-by-step guide to creating data and upload plugins | — |
| 09 | [Testing](09-testing.md) | Catch2 framework, test suites, coverage, how to add new tests | — |

---

## Quick Reference

### Architecture at a Glance

```
┌──────────────┐     ┌──────────────────────┐     ┌──────────────────┐
│  wxWidgets    │────▶│  cgui_core (Static)  │────▶│  Plugin System    │
│  GUI (Frame)  │     │  AppController        │     │  (.so / .dll)     │
│               │     │  ConfigManager        │     │  csv_input        │
│  Topic Combo  │     │  PluginLoader         │     │  json_input       │
│  Preview      │     │  TopicRegistry        │     │  db-pg_input      │
│  Validate     │     │  SchemaManager        │     │  db-ora_input     │
│  Upload       │     │  Validator            │     │  kafka_input      │
│  Log/Status   │     │  Uploader             │     │  hr_upload        │
└──────────────┘     └──────────────────────┘     └──────────────────┘
```

### Key Technologies

- **C++23** with `std::expected`, `std::format`, `std::filesystem`
- **wxWidgets 3.2+** for native desktop GUI
- **libsodium** for XChaCha20-Poly1305 encryption and Ed25519 signing
- **valijson** for JSON Schema validation
- **spdlog** for structured logging with rotating files
- **libcurl** for HTTPS uploads
- **Conan v2** + **CMake 3.28+** for cross-platform builds

### Build Commands

```bash
conan install . --output-folder=build --build=missing
cmake --preset conan-release
cmake --build --preset conan-release -j $(nproc)
cd build && ctest --output-on-failure
```

### Plugin Naming Convention

| Type | Pattern | Example |
|------|---------|---------|
| Data | `<topic>_<interface>_input.so` | `hr_csv_input.so` |
| Upload | `<topic>_upload.so` | `hr_upload.so` |

---

## Related Documents

- [Project README](../../README.md) — User-facing documentation
- [Architecture Overview (C4)](../architecture/architecture.md) — Original C4 model document
- [CHANGELOG](../../CHANGELOG.md) — Release history
- [encrypt_tool README](../encrypt_tool_readme.md) — Configuration encryption tool
