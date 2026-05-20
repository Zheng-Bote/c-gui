# Technical Documentation — Testing

---

## 1. Test Framework

c-gui uses **Catch2 v3** as its unit test framework. Tests are built as a single executable `unit_tests` that links against `cgui_core` and discovers tests at build time.

### Test Build Configuration

```cmake
# tests/CMakeLists.txt
find_package(Catch2 3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Dummy plugin for PluginLoader tests
add_library(dummy_plugin MODULE dummy_plugin.cpp)
target_include_directories(dummy_plugin PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(dummy_plugin PRIVATE nlohmann_json::nlohmann_json)

# Unit tests
add_executable(unit_tests
    test_main.cpp
    test_validator.cpp
    test_plugin_loader.cpp
    test_config_manager.cpp
    test_uploader.cpp
)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain cgui_core)
target_compile_definitions(unit_tests PRIVATE
    DUMMY_PLUGIN_PATH="$<TARGET_FILE:dummy_plugin>")

include(Catch)
catch_discover_tests(unit_tests)
```

---

## 2. Running Tests

```bash
# From build directory:
cd build
ctest --output-on-failure

# Run specific test:
./tests/unit_tests "[Validator]"

# List all tests:
./tests/unit_tests --list-tests
```

---

## 3. Test Suites

### 3.1 Validator Tests (`test_validator.cpp`)

Tests the JSON Schema validation logic:

```cpp
TEST_CASE("Validator validates JSON against schema", "[Validator]") {
    // Create schema
    nlohmann::json schema_json = R"({
        "$schema": "http://json-schema.org/draft-07/schema#",
        "type": "object",
        "properties": {
            "name": {"type": "string"},
            "age": {"type": "integer", "minimum": 0}
        },
        "required": ["name", "age"]
    })"_json;

    valijson::Schema schema;
    valijson::SchemaParser parser;
    parser.populateSchema(/* ... */);

    SECTION("Valid JSON") {
        nlohmann::json valid_doc = {{"name": "John Doe", "age": 30}};
        std::vector<ValidationError> errors;
        REQUIRE(Validator::validate(valid_doc, schema, errors) == true);
        REQUIRE(errors.empty());
    }

    SECTION("Invalid JSON - missing required field") { /* ... */ }
    SECTION("Invalid JSON - wrong type") { /* ... */ }
}
```

### 3.2 PluginLoader Tests (`test_plugin_loader.cpp`)

Tests dynamic loading of the dummy plugin:

```cpp
TEST_CASE("PluginLoader loads and unloads a plugin", "[PluginLoader]") {
    cgui::PluginLoader loader;
    std::string plugin_path = DUMMY_PLUGIN_PATH;
    REQUIRE(std::filesystem::exists(plugin_path));

    auto res = loader.load(plugin_path);
    REQUIRE(res.has_value());

    auto *plugin = loader.get_plugin();
    REQUIRE(plugin != nullptr);

    SECTION("Plugin can be initialized") {
        nlohmann::json config = {{"topic", "test_topic"}};
        REQUIRE(plugin->initialize(config) == true);
    }

    loader.unload();
}

TEST_CASE("PluginLoader handles non-existent plugin", "[PluginLoader]") {
    cgui::PluginLoader loader;
    auto res = loader.load("non_existent_path.so");
    REQUIRE(!res.has_value());
}
```

### 3.3 ConfigManager Tests (`test_config_manager.cpp`)

Tests encrypted INI loading with libsodium:

```cpp
TEST_CASE("ConfigManager loads and decrypts config", "[ConfigManager]") {
    ConfigManager manager;
    std::string password = "test_password";
    std::string ini_content = "[Section]\nkey=value\n";

    create_encrypted_ini(test_path, ini_content, password);
    auto result = manager.load_encrypted_ini(test_path, password);
    REQUIRE(result.has_value());

    const auto& config = manager.get_config();
    REQUIRE(config["Section"]["key"] == "value");

    SECTION("Fails with wrong password") {
        auto fail_result = manager.load_encrypted_ini(test_path, "wrong_password");
        REQUIRE(!fail_result.has_value());
    }
}
```

The test helper `create_encrypted_ini()` mirrors the `encrypt_tool` logic to create valid encrypted files.

### 3.4 Uploader Tests (`test_uploader.cpp`)

Tests the HTTP uploader with invalid URLs (no live network dependency):

```cpp
TEST_CASE("Uploader handles invalid URL", "[Uploader]") {
    Uploader uploader;
    nlohmann::json payload = {{"test", "data"}};

    SECTION("Synchronous upload fails with invalid URL") {
        auto result = uploader.upload_sync("https://invalid.example.com/api", payload, "token");
        REQUIRE(!result.has_value());
    }

    SECTION("Asynchronous upload returns error via callback") {
        bool callback_called = false;
        uploader.upload_async("https://invalid.example.com/api", payload,
            [&](std::expected<void, std::string> result) {
                callback_called = true;
                REQUIRE(!result.has_value());
            }, "token");
        // Wait for async thread
        int retries = 0;
        while (!callback_called && retries < 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            retries++;
        }
        REQUIRE(callback_called);
    }
}
```

---

## 4. Dummy Plugin

The `dummy_plugin.cpp` is a minimal test plugin used by `PluginLoader` tests:

- Topic: `test_topic`
- Interface: `dummy`
- Version: `0.1.0`
- `initialize()` always returns `true`
- `fetch_batch()` returns an empty vector

It is built as a `MODULE` (shared library) and its path is passed as a compile definition to the test executable via `$<TARGET_FILE:dummy_plugin>`.

---

## 5. Test Coverage Summary

| Component | Test File | Coverage |
|-----------|-----------|----------|
| Validator::validate() | `test_validator.cpp` | Valid/invalid docs, missing fields, type mismatches |
| PluginLoader::load() | `test_plugin_loader.cpp` | Successful load, metadata, failed load |
| PluginLoader::unload() | `test_plugin_loader.cpp` | Load → unload lifecycle |
| ConfigManager::load_encrypted_ini() | `test_config_manager.cpp` | Correct decryption, wrong password |
| Uploader::upload_sync() | `test_uploader.cpp` | Invalid URL handling |
| Uploader::upload_async() | `test_uploader.cpp` | Async callback error path |
| Uploader::set_timeout() | `test_uploader.cpp` | Configuration verification |

---

## 6. Adding New Tests

To add a new test:

1. Create a new `test_<component>.cpp` in `tests/`.
2. Add it to the `unit_tests` sources in `tests/CMakeLists.txt`.
3. Use Catch2 macros:

```cpp
#include <catch2/catch_test_macros.hpp>
#include "YourClass.hpp"

using namespace cgui;

TEST_CASE("Descriptive test name", "[YourClass]") {
    SECTION("Happy path") {
        // Arrange
        // Act
        // Assert
        REQUIRE(condition);
    }

    SECTION("Edge case") {
        // ...
    }
}
```

**Important:** Tests that use libsodium must call `sodium_init()` before any crypto operations.
