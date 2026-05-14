# c-gui Architecture Documentation

This document provides a comprehensive overview of the `c-gui` application architecture. It uses the [C4 model](https://c4model.com/) to describe the system at various levels of abstraction.

## Overview

`c-gui` is a modern C++23 desktop application designed for secure data validation and upload to SaaS endpoints. The application is built with a strong emphasis on security, modularity, and responsiveness.

**Key Technologies:**
- **Language:** C++23
- **GUI:** wxWidgets
- **Security:** libsodium (XChaCha20-Poly1305 for encryption)
- **Networking:** libcurl
- **Data Handling:** nlohmann/json
- **Plugin System:** ABI-stable C interface

---

## 1. Context Diagram

The Context diagram shows the `c-gui` application in its environment, illustrating its interactions with the user and external systems.

```mermaid
C4Context
    title System Context diagram for c-gui

    Person(user, "User", "A user of the c-gui application.")
    System(cgui, "c-gui Application", "Desktop application for data validation and upload.")
    
    System_Ext(datasource_csv, "Local File System", "Local CSV files.")
    System_Ext(datasource_db, "Database", "External PostgreSQL Database.")
    System_Ext(datasource_kafka, "Kafka Cluster", "External Kafka cluster.")
    System_Ext(saas_api, "SaaS Endpoint", "Target SaaS API for data upload.")

    Rel(user, cgui, "Configures and triggers processing, inputs password")
    Rel(cgui, datasource_csv, "Reads data from", "CSV Plugin")
    Rel(cgui, datasource_db, "Reads data from", "DB Plugin")
    Rel(cgui, datasource_kafka, "Reads data from", "Kafka Plugin")
    Rel(cgui, saas_api, "Uploads validated data to", "HTTPS/REST")
```

---

## 2. Container Diagram

The Container diagram breaks down the `c-gui` application into its major execution units (containers) and shows how they interact.

```mermaid
C4Container
    title Container diagram for c-gui

    Person(user, "User", "A user of the c-gui application.")
    
    System_Boundary(cgui_boundary, "c-gui Application") {
        Container(gui, "wxWidgets GUI", "C++23", "Provides the user interface for configuration, password entry, and monitoring progress.")
        Container(core, "cgui_core Library", "C++23 Static Lib", "Contains the core business logic, configuration management, and background task scheduling.")
        Container(plugin_system, "Plugin Interface", "C ABI", "Dynamically loads data source plugins at runtime.")
    }

    System_Ext(plugins, "Data Plugins", "Shared Libraries (.so/.dll)", "Implementations for CSV, DB, and Kafka data extraction.")
    System_Ext(saas_api, "SaaS Endpoint", "REST API", "The destination for validated data.")

    Rel(user, gui, "Uses", "GUI")
    Rel(gui, core, "Delegates processing to", "C++ Method Calls")
    Rel(core, plugin_system, "Loads and interfaces with", "ABI-stable C calls")
    Rel(plugin_system, plugins, "Dynamically links", "dlopen/LoadLibrary")
    Rel(core, saas_api, "Uploads data", "HTTPS/libcurl")
```

---

## 3. Component Diagram

The Component diagram zooms into the `cgui_core` library to show its internal structure and components.

```mermaid
C4Component
    title Component diagram for cgui_core

    Container_Boundary(core_boundary, "cgui_core Library") {
        Component(app_controller, "AppController", "C++ Class", "Orchestrates the overall flow of the application.")
        Component(config_manager, "ConfigManager", "C++ Class", "Handles decryption and management of INI settings via libsodium (crypto_aead_xchacha20poly1305_ietf).")
        Component(validator, "Validator", "C++ Class", "Validates incoming data against JSON schemas using nlohmann/json.")
        Component(uploader, "Uploader", "C++ Class", "Uploads validated data to the SaaS endpoint using libcurl.")
        Component(worker_threads, "Worker Threads", "std::jthread", "Executes background tasks for validation and uploading to keep the UI responsive.")
    }

    Component_Ext(gui, "wxWidgets GUI", "Front-end application window.")
    Component_Ext(plugins, "Plugins Interface", "ABI-stable C factory interface (`create_plugin`/`destroy_plugin`).")

    Rel(gui, app_controller, "Starts/Stops tasks")
    Rel(app_controller, config_manager, "Reads secure config")
    Rel(app_controller, worker_threads, "Dispatches background work to")
    Rel(worker_threads, plugins, "Fetches raw data via")
    Rel(worker_threads, validator, "Validates data with")
    Rel(worker_threads, uploader, "Uploads valid data via")
    Rel(worker_threads, gui, "Sends UI updates asynchronously via", "wxWindow::CallAfter")
```

---

## Key Architectural Decisions

### 1. Security First
- **No Plaintext Secrets:** Configuration files (`.ini`) containing API keys, database credentials, or SaaS endpoints are encrypted on disk using `libsodium`.
- **In-Memory Security:** The user is prompted for a master password at runtime. This password is used to decrypt the configuration into memory and is never persisted to disk.

### 2. Plugin-Based Data Extraction
- To support diverse and evolving data sources (e.g., flat files, SQL databases, message brokers like Kafka) without bloating the core application, data extraction is abstracted behind a plugin interface.
- Plugins are compiled as shared libraries (`.so` or `.dll`) and loaded at runtime. 
- The interface is defined using a pure C ABI (e.g., `create_plugin`, `destroy_plugin`) to ensure stability across different C++ compiler versions or standard libraries.

### 3. Responsive UI (Threading)
- Heavy operations such as reading data, validating large JSON payloads, and network I/O are strictly forbidden on the main thread.
- `std::jthread` is utilized to spawn worker threads that process data in the background.
- Communication back to the UI (e.g., progress bar updates, logging) is safely dispatched using `wxWidgets`'s `CallAfter` mechanism.
