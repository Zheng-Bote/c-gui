# Technical Documentation — Plugin Development Guide

---

## 1. Choosing a Plugin Type

| Feature | Native (C++) | WebAssembly (Go/TinyGo) |
| :--- | :--- | :--- |
| **Performance** | Maximum (Direct memory access) | High (Near-native execution) |
| **Security** | Full system access | Sandboxed (Limited resources) |
| **Portability** | OS-specific binaries | OS-independent modules |
| **Networking** | Direct Sockets/SSL | Host-assisted (REST only) |
| **Use Case** | Databases, Large Datasets | Data Mapping, APIs, Logic |

---

## 2. Developing WASM Plugins (Go)

The recommended language for WASM plugins is **Go** (with the `wasip1` target) or **TinyGo** for smaller binaries.

### 2.1 Interface Implementation

Your module must export the standard `c-gui` WASM API.

```go
// Memory allocation for host
//export allocate
func allocate(size int32) *byte {
    buf := make([]byte, size)
    return &buf[0]
}

// Data ingestion
//export fetch_batch
func fetch_batch(maxRecords int32) *byte {
    // Return JSON array as pointer to string
}
```

### 2.2 Using Host-Assisted Networking

Since WASI Preview 1 does not support direct sockets, use the host bridge for HTTP requests:

```go
//go:wasmimport env host_http_get
func host_http_get(urlPtr *byte, urlLen int32) *byte

//go:wasmimport env host_http_get_len
func host_http_get_len() int32
```

### 2.3 Compilation

```bash
GOOS=wasip1 GOARCH=wasm go build -o my_plugin.wasm main.go
```

---

## 3. Developing Native Plugins (C++)

Native plugins must implement the `IDataPlugin` or `IUploadPlugin` interfaces.

### 3.1 Factory Functions

Every native plugin must expose C-linkage entry points:

```cpp
extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new MyPlugin();
    }
    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
```

### 3.2 Build System Integration

Native plugins should be added as shared libraries in the `plugins/` subdirectory of the project.

---

## 4. Naming Convention

To be automatically discovered, plugins must follow this naming pattern:

- **Ingest**: `<topic>_<interface>_input` (e.g., `hr_csv_input.dll`)
- **Upload**: `<topic>_upload` (e.g., `hr_upload.so`)

*Note: Interface names for WASM plugins should match the extension-less filename suffix.*
