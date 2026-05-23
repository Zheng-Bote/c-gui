# Technical Documentation — Plugin System

---

## 1. Overview

**c-gui** uses a **Hybrid Plugin Architecture** that supports two types of ingest modules:
1.  **Native Plugins**: Compiled shared libraries (`.so` on Linux, `.dll` on Windows).
2.  **WebAssembly (WASM) Plugins**: Sandboxed modules (`.wasm`) executed via the integrated **Wasmtime** engine.

Both plugin types must adhere to a standardized interface, enabling the application to treat them interchangeably for data ingestion.

---

## 2. Hybrid Strategy

### 2.1 Native Plugins (Performance-Critical)
Native plugins are best suited for operations requiring:
- High-performance data processing.
- Direct access to local system drivers (e.g., Oracle Instant Client).
- Unlimited access to host system resources.

### 2.2 WASM Plugins (Portable & Secure)
WASM plugins are ideal for:
- Untrusted or third-party logic.
- Platform-independent data transformation.
- Ingestion from web services (REST APIs) via host-assisted networking.

---

## 3. WebAssembly Integration

The application hosts WASM modules using the **Wasmtime** engine. To overcome WASI Preview 1 limitations (lack of native socket support), the core application provides a **Host-Assisted Networking Bridge**.

### 3.1 Host Functions (`env` module)

WASM plugins can import the following functions from the host environment:

| Function | Signature (Guest) | Description |
| :--- | :--- | :--- |
| `host_http_get` | `(ptr: i32, len: i32) -> i32` | Triggers a native `libcurl` GET request. Returns a pointer to the response in WASM memory. |
| `host_http_get_len` | `() -> i32` | Returns the length of the last HTTP response. |
| `allocate` | `(size: i32) -> i32` | (Provided by Guest) Allows Host to reserve memory for data transfers. |

### 3.2 Bidirectional Communication

Communication between the C++ host and WASM modules is performed via **JSON strings** exchanged through shared linear memory.

1.  **Initialize**: Host passes a JSON configuration string (including proxies and connection settings).
2.  **Fetch**: Guest returns a JSON array of records.
3.  **Metadata**: Guest provides version and topic information via exported string functions.

---

## 4. Plugin Interface (C API)

For native plugins, the following C-linkage functions must be exported:

```cpp
extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin();
    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin);
}
```

For WASM plugins, the module must export:
- `allocate(size: i32) -> i32`
- `initialize(ptr: i32, len: i32) -> i32`
- `fetch_batch(max: i32) -> i32`
- `get_topic() -> i32`
- `get_version() -> i32`

---

## 5. Directory Structure & Discovery

The `AppController` scans the following directories at runtime:
- `./plugins/data/`: Ingest plugins (Native & WASM).
- `./plugins/upload/`: Upload plugins (Native).

The discovery logic supports:
- Automatic extension detection (`.dll`, `.so`, `.wasm`).
- Metadata extraction (Topic, Version, Interface Type) during scan.
- Exception-safe loading (malformed plugins are logged and skipped).
