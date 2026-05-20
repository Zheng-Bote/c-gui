# Technical Documentation — Plugin Development Guide

This guide explains how to create new data or upload plugins for c-gui.

---

## 1. Plugin Structure Overview

Each plugin lives in its own directory under `plugins/` and must contain:

- A header file declaring the plugin class
- A source file implementing the class + C factory functions
- A `CMakeLists.txt` for the build

### Minimal Example (from `dummy_plugin.cpp`)

```cpp
#include "PluginAPI.hpp"
#include <string>

class DummyPlugin : public cgui::IDataPlugin {
public:
    std::string get_topic() const override { return "test_topic"; }
    std::string get_interface_type() const override { return "dummy"; }
    std::string get_version() const override { return "0.1.0"; }
    bool initialize(const nlohmann::json& config) override { (void)config; return true; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override { (void)max_records; return {}; }
    void shutdown() override {}
};

extern "C" {
    cgui::IPlugin* create_plugin() { return new DummyPlugin(); }
    void destroy_plugin(cgui::IPlugin* plugin) { delete plugin; }
}
```

---

## 2. Creating a Data Plugin

### When to create a data plugin

Create a data plugin when you need to ingest data from a new source type for an existing or new topic. Each data plugin maps one (topic, interface) pair.

### Step-by-Step

**Step 1: Create the directory**

```
plugins/<new_topic>_<interface>_input/
  CMakeLists.txt
  <PluginName>.hpp
  <PluginName>.cpp
```

**Step 2: Define the class**

```cpp
// MyNewInputPlugin.hpp
#ifndef CGUI_MY_NEW_INPUT_PLUGIN_HPP
#define CGUI_MY_NEW_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>
#include <vector>

namespace cgui {

class MyNewInputPlugin : public IDataPlugin {
public:
    MyNewInputPlugin() = default;
    ~MyNewInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override;
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    [[nodiscard]] std::string get_interface_type() const override;
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_topic;
    std::string m_source_path;
    bool m_initialized = false;
};

} // namespace cgui

#endif // CGUI_MY_NEW_INPUT_PLUGIN_HPP
```

**Step 3: Implement the class**

```cpp
// MyNewInputPlugin.cpp
#include "MyNewInputPlugin.hpp"

namespace cgui {

bool MyNewInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("topic") || !config.contains("file_path")) {
        return false;
    }
    m_topic = config["topic"].get<std::string>();
    m_source_path = config["file_path"].get<std::string>();
    m_initialized = true;
    return true;
}

std::string MyNewInputPlugin::get_topic() const {
    return m_topic;
}

std::string MyNewInputPlugin::get_interface_type() const {
    return "myformat";  // must match the interface name used in file naming and INI
}

std::vector<nlohmann::json> MyNewInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized) return batch;

    // TODO: Read from m_source_path, parse into JSON objects
    // Limit to max_records per batch

    return batch;
}

void MyNewInputPlugin::shutdown() {
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::MyNewInputPlugin();
    }
    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
```

**Step 4: Create CMakeLists.txt**

```cmake
# plugins/myformat_input/CMakeLists.txt
add_library(my_topic_myformat_input MODULE
    MyNewInputPlugin.cpp
)

target_include_directories(my_topic_myformat_input PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(my_topic_myformat_input PRIVATE
    nlohmann_json::nlohmann_json
    # Add any additional libraries your plugin needs
)
```

**Step 5: Register in plugins/CMakeLists.txt**

```cmake
add_subdirectory(my_topic_myformat_input)
```

**Step 6: Add INI configuration**

```ini
[topics.MyTopic]
data_myformat = "./path/to/default/data"
```

---

## 3. Creating an Upload Plugin

### When to create an upload plugin

Create an upload plugin when a topic needs a specialized upload handshake, payload wrapping, or authentication flow.

### Step-by-Step

```cpp
// MyUploadPlugin.hpp
#include "PluginAPI.hpp"
#include <string>

namespace cgui {

class MyUploadPlugin : public IUploadPlugin {
public:
    MyUploadPlugin() = default;
    ~MyUploadPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    std::string get_topic() const override;
    std::string get_version() const override { return "1.0.0"; }
    std::expected<void, std::string> upload(const nlohmann::json& data) override;
    void shutdown() override;

private:
    std::string m_endpoint;
    std::string m_bearer_token;
};
```

Key implementation details for a generic upload plugin:

```cpp
std::expected<void, std::string> MyUploadPlugin::upload(const nlohmann::json& data) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("curl init failed");

    std::string response_string;
    std::string payload = data.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!m_bearer_token.empty()) {
        std::string auth = "Authorization: Bearer " + m_bearer_token;
        headers = curl_slist_append(headers, auth.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, m_endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return std::unexpected(curl_easy_strerror(res));
    if (http_code < 200 || http_code >= 300)
        return std::unexpected("HTTP " + std::to_string(http_code));

    return {};
}
```

---

## 4. Naming Convention Summary

```
Data Plugin:   <topic>_<interface>_input     → hr_csv_input
Upload Plugin: <topic>_upload                → hr_upload
Library File:  <plugin_name>.so|.dll         → hr_csv_input.so
```

The topic and interface are case-insensitive (lowercased during discovery).

---

## 5. Configuration Keys Passed to Plugin

### Data Plugin Config

```json
{
    "topic": "HR",
    "file_path": "/path/to/data.csv",
    "connection_string": "host=... port=...",
    "...": "any additional keys from INI"
}
```

### Upload Plugin Config

```json
{
    "topic": "HR",
    "upload_endpoint": "https://saas.example.com/api/employeeimport",
    "base_url": "https://saas.example.com",
    "login": "saas_user",
    "password": "encrypted_secret",
    "proxy": "user:pass@proxy:8080",
    "options": {
        "dateFormat": "dd.mm.yyyy",
        "updateExistingRecords": "true"
    },
    "auth": {
        "saas_base_url": "...",
        "saas_login": "...",
        "saas_password": "...",
        "verify_ssl": "true"
    },
    "verify_ssl": "true",
    "ssl_ca_path": "./certs/cacert.pem",
    "upload_timeout": "60"
}
```

---

## 6. Export Macro

Always use the platform-appropriate export macro:

```cpp
#ifdef _WIN32
#define CGUI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define CGUI_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif
```

---

## 7. Testing Your Plugin

1. Build the entire project (plugins are built as part of the CMake build).
2. Place your `.so`/`.dll` in the appropriate `build/plugins/data/` or `build/plugins/upload/` directory.
3. Run `c-gui` and check the Manage Plugins dialog to see your plugin listed.
4. Select the corresponding topic and interface to load data.

For automated testing, see `tests/dummy_plugin.cpp` as a reference for creating a test plugin that can be linked into unit tests.

---

## 8. Best Practices

- **Error handling**: All `initialize()` and `fetch_batch()` / `upload()` functions should fail safely.
- **Memory**: The plugin instance is heap-allocated and destroyed via `delete` — ensure correct virtual destructor.
- **Thread safety**: `AppController` loads plugins sequentially; but upload background threads create their own PluginLoader. Do not rely on global state.
- **Logging**: Use `LogManager::get_instance().get_logger(topic)` for topic-specific logging inside plugins.
- **File paths**: Use `std::filesystem::path` for cross-platform path handling.
- **Avoid exceptions**: Prefer `std::expected` return values where possible. If exceptions are unavoidable, catch them in `fetch_batch()` / `upload()`.
- **Plugin size**: Keep plugins focused on a single data source. Complex transformation logic should live in the upload plugin.
