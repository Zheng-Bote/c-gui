# Technical Documentation — C4 Container & Component Architecture

---

## 1. Container Diagram (C4 Level 2)

The application is decomposed into three primary execution units: the **wxWidgets GUI**, the **cgui_core static library**, and the **Plugin Loader** subsystem.

```mermaid
C4Container
    title Container diagram for c-gui

    Person(user, "User", "Operates the application")

    System_Boundary(cgui_boundary, "c-gui Application") {

        Container(gui, "wxWidgets GUI", "C++23 / wxFrame", "Renders the main window with topic selection, date format controls, data preview list, progress gauge, and log area. Dispatches user actions to AppController.")

        Container(core, "cgui_core Library", "C++23 Static Library", "Contains all business logic: configuration management, topic registry, schema caching, data validation, HTTP upload, and logging. Orchestrated by AppController.")

        Container(plugin_system, "Plugin Loader", "C ABI / dlopen", "Dynamically discovers and loads shared libraries conforming to the IPlugin C++ interface. Uses exported C factory functions create_plugin() and destroy_plugin().")

        Container(enc_tool, "encrypt_tool", "C++23 CLI", "Standalone command-line tool to encrypt plaintext INI files into the binary .enc format consumed by ConfigManager.")
    }

    System_Ext(data_plugins, "Data Plugins", "Shared Libraries (.so/.dll)", "Naming convention: <topic>_<interface>_input")
    System_Ext(upload_plugins, "Upload Plugins", "Shared Libraries (.so/.dll)", "Naming convention: <topic>_upload")
    System_Ext(saas_api, "SaaS Endpoint", "REST API", "Target system for data upload")
    System_Ext(github, "GitHub", "HTTPS JSON", "Release version check")

    Rel(user, gui, "Selects topic/interface/format, clicks Load/Validate/Upload")
    Rel(gui, core, "Calls AppController methods", "C++ direct calls")
    Rel(core, plugin_system, "Loads/unloads plugins via PluginLoader")
    Rel(plugin_system, data_plugins, "dlopen / LoadLibrary")
    Rel(plugin_system, upload_plugins, "dlopen / LoadLibrary")
    Rel(data_plugins, core, "Returns vector<nlohmann::json>")
    Rel(core, upload_plugins, "Passes config + JSON data")
    Rel(upload_plugins, saas_api, "POST with Bearer token", "libcurl HTTPS")
    Rel(core, github, "Checks latest release version (async)")
    Rel(user, enc_tool, "Runs CLI to encrypt INI", "shell")
```

---

## 2. Component Diagram — cgui_core Library (C4 Level 3)

```mermaid
C4Component
    title Component diagram for cgui_core

    Container_Boundary(core_boundary, "cgui_core Library") {

        Component(app_ctrl, "AppController", "C++ Class", "Orchestrates the application lifecycle. Manages topic selection, date format propagation, plugin discovery, data loading, background validation, and background upload threads.")

        Component(cfg_mgr, "ConfigManager", "C++ Class", "Loads encrypted .enc file, derives key via Argon2id, decrypts with XChaCha20-Poly1305, parses INI, and exposes a const nlohmann::json& configuration tree.")

        Component(plugin_loader, "PluginLoader", "C++ Class", "Manages a single loaded plugin instance at a time. Calls dlopen/dlsym or LoadLibrary/GetProcAddress to resolve create_plugin/destroy_plugin symbols.")

        Component(topic_reg, "TopicRegistry", "C++ Class", "In-memory map of registered topics with their metadata: schema path, upload endpoint, upload plugin name, proxy, and timeout.")

        Component(schema_mgr, "SchemaManager", "C++ Class", "Loads JSON Schema files via valijson, parses them into valijson::Schema objects, and caches them by filesystem path for reuse across validation runs.")

        Component(validator, "Validator", "C++ Class (static)", "Stateless wrapper around valijson::Validator. Takes a JSON document and a Schema, returns a vector of ValidationError structs.")

        Component(uploader, "Uploader", "C++ Class", "Manages libcurl easy handles for HTTP POST. Supports sync and async uploads, SSL verification toggle, proxy configuration, and timeout control.")

        Component(auth_mgr, "AuthManager", "C++ Class (static)", "Implements the Cority OAuth2-like flow: POST to /api/refreshtoken -> GET /api/token/ with Bearer refresh token -> returns access token with expiry.")

        Component(log_mgr, "LogManager", "C++ Singleton", "Creates and manages per-topic spdlog loggers with rotating file sinks, console sinks, a GUI callback sink, and a custom audit signing sink for Ed25519 log signatures.")
    }

    Component_Ext(gui, "MainWindow (GUI)", "wxFrame", "UI event handlers and display components")
    Component_Ext(plugins, "Plugin Shared Libraries", ".so / .dll", "IDataPlugin / IUploadPlugin implementations")
    Component_Ext(build_cfg, "rz_config.hpp", "Generated Header", "Version, copyright, compiler info constants")

    Rel(gui, app_ctrl, "Calls init(), select_topic(), load_data(), start_validation(), start_upload()")
    Rel(app_ctrl, cfg_mgr, "Reads config JSON for paths, auth, topics, logging, proxy")
    Rel(app_ctrl, plugin_loader, "Loads plugin for data source or upload")
    Rel(app_ctrl, topic_reg, "Registers topics from config, retrieves TopicMeta")
    Rel(app_ctrl, schema_mgr, "Requests Schema by path")
    Rel(app_ctrl, validator, "Validates each record in background thread", "static call")
    Rel(app_ctrl, uploader, "Fallback upload (legacy; plugins preferred)")
    Rel(app_ctrl, log_mgr, "Initializes logging, gets topic loggers, signs audit logs")
    Rel(app_ctrl, build_cfg, "Reads VERSION and PROJECT_NAME")
    Rel(plugins, validator, "Validates data (called from AppController, not plugin)")
    Rel(topic_reg, cfg_mgr, "Topic metadata derived from config")
    Rel(schema_mgr, cfg_mgr, "Schema path from TopicMeta")
    Rel(uploader, plugins, "Plugins preferred in v0.3.0+")
```

---

## 3. GUI Layout (MainWindow)

```
                     Menu Bar
   File              Manage            Help
   [Exit]           [Plugins...]       [About]

                     Top Bar
   [Topic: v]   [Interface: v]   [Load...]

                   Date Format Bar
   [Date Format: v]   [Delimiter: v]

                 Data Preview (wxListCtrl)
   First 100 records shown as table with dynamic columns

                   Button Bar
   [Validate]            [Upload]

                   Progress
   [==========wxGauge==========]

                   Log / Status (wxTextCtrl)
   Multi-line read-only log output
```

### 3.1 Event Flow

```mermaid
sequenceDiagram
    participant U as User
    participant MW as MainWindow
    participant AC as AppController
    participant PL as PluginLoader
    participant PLUG as Plugin

    U->>MW: Select topic
    MW->>AC: select_topic(topic)
    AC->>MW: update_log()
    MW->>AC: get_available_interfaces(topic)
    AC->>AC: Scan plugin directory for topic_*_input.so
    AC-->>MW: interfaces list
    MW->>MW: Populate interface choice

    U->>MW: Click Load...
    MW->>MW: Show file dialog / text entry
    U->>MW: Select file/connection
    MW->>AC: load_data(interface, path)
    AC->>PL: load(plugin_path)
    PL->>PLUG: dlopen + create_plugin()
    PLUG-->>PL: IDataPlugin*
    AC->>PLUG: initialize(config)
    AC->>PLUG: fetch_batch(1000000)
    PLUG-->>AC: vector<nlohmann::json>
    AC->>PLUG: shutdown()
    AC-->>MW: update_preview() + update_buttons(true, false)

    U->>MW: Click Validate
    MW->>AC: start_validation()
    AC->>AC: Background thread loops records
    AC-->>MW: on_validation_complete() + update_buttons(true, true)

    U->>MW: Click Upload
    MW->>AC: start_upload()
    AC->>AC: Background thread loads upload plugin
    AC->>PLUG: upload(data)
    PLUG->>SaaS: HTTPS POST with Bearer token
    PLUG-->>AC: expected<void, string>
    AC-->>MW: on_upload_complete() + audit log
```

---

## 4. Threading Model

```
Main Thread (wx)
-----------------
wxApp::OnInit() -> CGuiApp
  - AppController::init()
  - MainWindow constructor (GUI controls)
  - wxEventLoop (event handlers)

Event handlers run here:
  - on_topic_selected
  - on_load_data      (blocking plugin call)
  - on_about          (detaches update check thread)
  - on_date_format_changed
  - on_manage_plugins (blocking scan)

GUI updates via CallAfter():
  - update_log()          - safe from any thread
  - update_progress()     - safe from any thread
  - update_preview()      - safe from any thread
  - update_buttons()      - safe from any thread


Background Thread: Validation
-------------------------------
std::thread([] { ... }).detach()
  - Iterates m_current_data[] records
  - Calls Validator::validate(record, schema)
  - Reports progress via CallAfter() every 100 records
  - Limits errors to 1000 for UI safety
  - Calls on_validation_complete() when done


Background Thread: Upload
-------------------------------
std::thread([] { ... }).detach()
  - Creates separate PluginLoader instance
  - Loads upload plugin (.so/.dll)
  - Merges topic-specific options from config
  - Calls IUploadPlugin::upload(data)
  - Plugin performs Cority auth flow internally
  - Calls on_upload_complete() when done
  - Audit log entry written (signed with Ed25519)


Background Thread: Update Check
----------------------------------
std::thread([] { ... }).detach()
  - Calls ghupdate::check_github_update_async()
  - Returns std::future<ghupdate::UpdateResult>
  - Shows About dialog with update info via CallAfter()
```

**Safety invariants:**
- m_current_data, m_preview_data, m_current_topic are mutated only from the main thread before background threads are spawned.
- Background threads read these values (copied vectors, not shared references) -- no mutex needed.
- GUI updates use CallAfter() which queues the lambda on the wx event loop.
- Each background thread loads its own PluginLoader instance for upload (separate dlopen).
