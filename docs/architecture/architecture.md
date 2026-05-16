# c-gui Architecture Documentation

This document provides a comprehensive overview of the `c-gui` application architecture (v0.3.0). It uses the [C4 model](https://c4model.com/) to describe the system at various levels of abstraction.

## Overview

`c-gui` is a modern C++23 desktop application designed for secure data validation and upload to SaaS endpoints. The application is built with a strong emphasis on security, modularity, and responsiveness.

**Key Technologies:**
- **Language:** C++23
- **GUI:** wxWidgets
- **Security:** libsodium (XChaCha20-Poly1305 for encryption)
- **Networking:** libcurl
- **Data Handling:** nlohmann/json
- **Plugin System:** ABI-stable C interface with dynamic discovery
- **Update Checker:** GitHub API integration (v1.1.0, async)

---

## 1. Context Diagram

The Context diagram shows the `c-gui` application in its environment, illustrating its interactions with the user and external systems.

```mermaid
C4Context
    title System Context diagram for c-gui

    Person(user, "User", "A user of the c-gui application.")
    System(cgui, "c-gui Application", "Desktop application for data validation and upload.")
    
    System_Ext(datasource_any, "Data Sources", "CSV, JSON, PostgreSQL, Oracle, Kafka, etc.")
    System_Ext(saas_api, "SaaS Endpoint", "Target SaaS API for data upload.")
    System_Ext(github, "GitHub", "Project repository for update checks.")

    Rel(user, cgui, "Configures and triggers processing, inputs password, selects date format")
    Rel(user, cgui, "Selects JSON/CSV files via dialog")
    Rel(cgui, datasource_any, "Reads data from", "Dynamic Data Plugin")
    Rel(cgui, saas_api, "Uploads validated data to", "Dynamic Upload Plugin (HTTPS/REST)")
    Rel(cgui, github, "Checks for updates (Async)", "HTTPS/JSON")
```

---

## 2. Container Diagram

The Container diagram breaks down the `c-gui` application into its major execution units (containers) and shows how they interact.

```mermaid
C4Container
    title Container diagram for c-gui

    Person(user, "User", "A user of the c-gui application.")
    
    System_Boundary(cgui_boundary, "c-gui Application") {
        Container(gui, "wxWidgets GUI", "C++23", "Provides the user interface for topic/interface selection, password entry, date format config, and monitoring.")
        Container(core, "cgui_core Library", "C++23 Static Lib", "Contains the core business logic, configuration management, and background task scheduling.")
        Container(plugin_system, "Plugin Loader", "C ABI", "Dynamically discovers and loads specialized plugins at runtime.")
    }

    System_Ext(data_plugins, "Data Plugins", "Shared Libs", "Naming: <topic>_<type>_input.[so|dll]")
    System_Ext(upload_plugins, "Upload Plugins", "Shared Libs", "Naming: <topic>_upload.[so|dll]")
    System_Ext(saas_api, "SaaS Endpoint", "REST API", "The destination for validated data.")

    Rel(user, gui, "Uses", "GUI")
    Rel(gui, core, "Delegates processing to", "C++ Method Calls")
    Rel(core, plugin_system, "Discovers and loads", "dlopen/LoadLibrary")
    Rel(plugin_system, data_plugins, "Fetches raw data")
    Rel(plugin_system, upload_plugins, "Transmits data")
    Rel(upload_plugins, saas_api, "Uploads data", "HTTPS/libcurl")
```

---

## 3. Component Diagram

The Component diagram zooms into the `cgui_core` library to show its internal structure and components.

```mermaid
C4Component
    title Component diagram for cgui_core

    Container_Boundary(core_boundary, "cgui_core Library") {
        Component(app_controller, "AppController", "C++ Class", "Orchestrates the overall flow, manages session lifecycle, merges topic-specific options, and handles date formatting.")
        Component(config_manager, "ConfigManager", "C++ Class", "Handles decryption and management of INI settings (with quote stripping).")
        Component(log_manager, "LogManager", "C++ Class", "Manages topic-based spdlog instances with a custom GUI callback sink.")
        Component(validator, "Validator", "C++ Class", "Validates incoming data against JSON schemas using nlohmann/json.")
        Component(uploader, "Uploader", "C++ Class", "Legacy/Generic upload utility (plugins preferred in v0.3.0).")
        Component(gh_update, "GH Update Checker", "FetchContent", "Checks for new releases on GitHub (v1.1.0, asynchronous).")
    }

    Component_Ext(gui, "wxWidgets GUI", "Front-end application window with date format controls.")
    Component_Ext(plugins, "Plugins", "ABI-stable C factory interface.")

    Rel(gui, app_controller, "Starts/Stops tasks, sets date format")
    Rel(app_controller, config_manager, "Reads secure config & topic-specific options")
    Rel(app_controller, log_manager, "Registers GUI log sink")
    Rel(app_controller, plugins, "Discovers and executes with merged options")
    Rel(plugins, validator, "Validates data with")
    Rel(gui, gh_update, "Triggered via About dialog (non-blocking)")
```

---

## Key Architectural Decisions

### 1. Dual-Plugin Strategy (Mandatory)
To support diverse data sources and complex SaaS API handshakes, each topic MUST be split into two specialized plugins:
- **Data (Ingest) Plugins (`<topic>_<type>_input`):** Handle reading from specific sources (CSV, JSON, DB-Ora, DB-PG, Kafka) and converting to the internal JSON format.
- **Upload (Output) Plugins (`<topic>_upload`):** Handle topic-specific API handshakes, payload formatting (wrapping), and secure transmission.

### 2. Topic-Specific Payload Overrides (v0.3.0)
The application now supports overriding the upload payload options per topic via the INI file. The `AppController` merges global settings with `[topics.<TopicName>.options]` before initializing the upload plugin.

### 3. Flexible Date Formatting (v0.3.0)
The GUI allows users to select date component order and delimiters. These settings are synchronized with topic-specific INI defaults and passed to the upload plugin, which prioritizes them over hardcoded defaults.

### 4. Non-Blocking Update Checks (v1.1.0 Lib)
By utilizing the asynchronous API of the `gh-update-checker` library, update checks no longer freeze the UI thread, ensuring a modern and responsive user experience.

