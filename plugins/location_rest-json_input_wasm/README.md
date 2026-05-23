# Location REST-JSON WASM Ingest Plugin (Go)

This plugin retrieves location data from a remote REST API. It demonstrates how WASM plugins can perform networking operations by utilizing host-assisted HTTP functions provided by the `c-gui` application.

## Features

- **Networking Support**: Uses host-side `libcurl` to fetch data (overcoming WASI Preview 1 limitations).
- **Dynamic URL**: Retrieves the API endpoint from the application configuration.
- **Secure**: Networking is controlled and logged by the host application.

## Prerequisites

- **Go**: Version 1.21 or newer.
- **Target**: `wasip1` (WASI Preview 1).
- **Build Tool**: Standard Go compiler or [TinyGo](https://tinygo.org/).

## Configuration (INI File)

The plugin expects the REST URL to be configured under the `[topics.Location]` section.

```ini
[topics.Location]
schema_path = "data/location_schema.json"
upload_endpoint = "https://internal.cority.com/ingest"

# REST API Endpoint for data retrieval
# Interface name: rest-json
data_rest-json = "https://api.world-cities.com/v1/locations"
```

## Compilation

```bash
cd plugins/location_rest-json_input_wasm

# Build as WASM module
GOOS=wasip1 GOARCH=wasm go build -o location_rest-json_input.wasm main.go
# or
tinygo build -o ..\..\build\build\plugins\Release\location_rest-json_input.wasm -target=wasi main.go
```

Move the resulting `location_rest-json_input.wasm` to your `./plugins/data/` folder.

## How it Works

1.  **Host Import**: The plugin imports `host_http_get` and `host_http_get_len` from the `env` module provided by the `c-gui` core.
2.  **Configuration**: During `initialize`, the plugin stores the URL provided via the `data_rest-json` key.
3.  **Data Fetch**: When `fetch_batch` is called, the plugin requests the host to perform the HTTP GET request. The host uses its native `libcurl` implementation and copies the response into the plugin's memory.
4.  **Parsing**: The plugin validates the received JSON before passing it back to the core application.
