# Plugin Development Guide (v0.2.0)

This guide describes how to create, build, and deploy new plugins for the `c-gui` application.

## 1. Plugin Types

The application follows a **Dual-Plugin Strategy**. Every data topic requires two types of plugins:

### Data (Ingest) Plugins
- **Purpose**: Read data from a source and convert it to the internal JSON format.
- **Naming Convention**: `<topic>_<interface>_input.[so|dll]` (e.g., `hr_csv_input.so`).
- **Interface**: `cgui::IDataPlugin`.

### Upload (Output) Plugins
- **Purpose**: Restructure the JSON payload for a specific SaaS API and handle the transmission.
- **Naming Convention**: `<topic>_upload.[so|dll]` (e.g., `hr_upload.so`).
- **Interface**: `cgui::IUploadPlugin`.

---

## 2. API Implementation

All plugins must implement the base `cgui::IPlugin` interface and its specialized child interfaces defined in `include/PluginAPI.hpp`.

### Base Interface Methods (`IPlugin`)
- `bool initialize(const nlohmann::json& config)`: Called after loading. Use this to setup connections or open files.
- `std::string get_topic() const`: Must return the topic name (e.g., "HR").
- `std::string get_version() const`: Must return the plugin version (e.g., "1.0.0").
- `PluginType get_type() const`: Handled by child interfaces (`DATA` or `UPLOAD`).
- `void shutdown()`: Called before unloading. Cleanup all resources here.

### Data Plugin Methods (`IDataPlugin`)
- `std::string get_interface_type() const`: Return the short name of the input method (e.g., "csv", "db-pg").
- `std::vector<nlohmann::json> fetch_batch(size_t max_records)`: Read data and return it as a vector of JSON objects.

### Upload Plugin Methods (`IUploadPlugin`)
- `std::expected<void, std::string> upload(const nlohmann::json& data)`: Perform topic-specific payload wrapping and transmission.

---

## 3. Factory Interface

Plugins must export a C-compatible factory interface for ABI stability:

```cpp
#include "PluginAPI.hpp"

class MyDataPlugin : public cgui::IDataPlugin { /* ... */ };

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new MyDataPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
```

---

## 4. Build System (CMake)

Every plugin should be its own shared library. Here is a template `CMakeLists.txt`:

```cmake
add_library(hr_myinterface_input SHARED MyPlugin.cpp MyPlugin.hpp)

target_include_directories(hr_myinterface_input PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find dependencies (e.g., nlohmann_json, libcurl, etc.)
find_package(nlohmann_json REQUIRED)
target_link_libraries(hr_myinterface_input PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(hr_myinterface_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
```

---

## 5. Deployment

1. Compile the plugin shared library.
2. Place the file in the directory configured in `sample.ini` (typically `./plugins/data` or `./plugins/upload`).
3. Ensure the filename matches the convention: `<topic>_<interface>_input.so` or `<topic>_upload.so`.
4. Restart the application. The plugin will be automatically discovered when the topic is selected.

### UI Selection
- If you create a Data Plugin named `hr_myinterface_input.so`, the user will see "**myinterface**" in the Interface dropdown when the "**HR**" topic is selected.
- Selecting "**myinterface**" and clicking "**Load...**" will trigger your plugin's `initialize` and `fetch_batch` methods.
