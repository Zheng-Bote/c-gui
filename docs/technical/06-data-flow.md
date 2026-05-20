# Technical Documentation — Data Flow

---

## 1. End-to-End Data Flow

```mermaid
flowchart LR
    subgraph User Actions
        A[Select Topic]
        B[Select Interface]
        C[Select File/Connection]
        D[Click Validate]
        E[Click Upload]
    end

    subgraph System
        F[Plugin Discovery]
        G[Data Ingestion]
        H[JSON Schema\nValidation]
        I[SaaS Upload]
    end

    subgraph Data Formats
        J[CSV / JSON / XLSX]
        K[DB Query Result]
        L[Kafka Message]
        M[JSON Records Array]
        N[Validated JSON]
        O[Upload Payload]
    end

    A --> F
    F --> B
    B --> C
    C --> G

    G --> J
    G --> K
    G --> L

    J --> M
    K --> M
    L --> M

    D --> H
    M --> H
    H --> N

    E --> I
    N --> I
    I --> O
    O --> SaaS[(SaaS Endpoint)]
```

---

## 2. Data Ingestion Pipeline

```mermaid
sequenceDiagram
    participant MW as MainWindow
    participant AC as AppController
    participant PL as PluginLoader
    participant PLUG as IDataPlugin
    participant FILE as Data Source

    MW->>MW: on_load_data()
    MW->>MW: Show FileDialog or TextEntryDialog

    Note over MW: User selects CSV file

    MW->>AC: load_data("csv", "/path/to/file.csv")
    AC->>AC: build plugin name: "hr_csv_input"
    AC->>PL: load("./plugins/data/hr_csv_input.so")

    PL->>PLUG: dlopen + create_plugin()
    PL-->>AC: IPlugin*

    AC->>PLUG: initialize({"topic":"HR","file_path":"/path/to/file.csv"})
    PLUG->>FILE: Open file, read headers
    PLUG-->>AC: true

    AC->>PLUG: fetch_batch(1000000)
    PLUG->>PLUG: Parse CSV lines into JSON objects
    PLUG-->>AC: [{EmployeeNumber, FirstName, LastName, ...}, {...}]

    AC->>PLUG: shutdown()
    AC->>AC: store in m_current_data
    AC->>AC: create m_preview_data (first 100 records)

    AC-->>MW: update_preview()
    AC-->>MW: update_buttons(true, false)
```

### 2.1 Plugin-Native Data Formats

Each data plugin converts its source to the common internal format: `std::vector<nlohmann::json>` where each element is a JSON object with string-valued fields.

| Plugin | Source Format | Internal Conversion |
|--------|-------------|-------------------|
| CSV | `file.csv` | Parse lines by comma, first line as headers |
| JSON | `file.json` | Parse as `nlohmann::json`, expect array of objects |
| XLSX | `file.xlsx` | OpenXLSX row iterator, first row as headers |
| PostgreSQL | libpqxx query result | Column names as keys, row values as strings |
| Oracle | connection string | Abstraction layer (future) |
| SQLite | file path | SQLite C API (future) |
| Kafka | broker + topic | librdkafka consumer (stub) |
| S3/MFT | S3 bucket path | Stub |

---

## 3. Validation Pipeline

```mermaid
flowchart TB
    subgraph Main Thread
        A["start_validation()"]
        B[Get TopicMeta from registry]
        C[Get Schema from SchemaManager]
        D[Spawn background thread]
    end

    subgraph Background Thread
        E[Loop: for each record in m_current_data]
        F["Validator::validate(record, schema)"]
        G{Valid?}
        H[Collect error]
        I[Update progress every 100 records]
        J["Call on_validation_complete()"]
    end

    subgraph Post-Validation
        K{All valid?}
        L[Enable Upload button]
        M[Show errors in log]
        N[Disable Upload button]
    end

    A --> B --> C --> D
    D --> E
    E --> F
    F --> G
    G -->|Yes| I
    G -->|No| H --> I
    I --> E
    E -->|Done| J
    J --> K
    K -->|Yes| L
    K -->|No| M --> N
```

### 3.1 Schema Validation Details

- The JSON Schema is loaded once and cached in `SchemaManager` (keyed by path).
- `valijson::SchemaParser::populateSchema()` converts the JSON Schema into the runtime representation.
- Each record is validated independently via `valijson::Validator::validate()`.
- Errors are limited to 1000 to avoid overwhelming the UI log.
- Progress is reported via `CallAfter()` every 100 records.

### 3.2 Sample Schema (HR)

```json
{
  "type": "object",
  "properties": {
    "EmployeeNumber": { "type": "string", "pattern": "^[0-9]{6,8}$" },
    "FirstName": { "type": "string", "minLength": 2, "maxLength": 255 },
    "LastName": { "type": "string", "minLength": 2, "maxLength": 255 },
    "DateOfBirth": { "type": "string", "pattern": "^\\d{2}\\.\\d{2}\\.\\d{4}$" },
    "Email": { "type": "string" }
  },
  "required": ["EmployeeNumber", "FirstName", "LastName"]
}
```

### 3.3 ValidationError Struct

```cpp
struct ValidationError {
    std::string path;     // e.g., "[Record 5] /FirstName"
    std::string message;  // e.g., "value is less than minimum length of 2"
};
```

---

## 4. Upload Pipeline

```mermaid
sequenceDiagram
    participant MW as MainWindow
    participant AC as AppController
    participant TR as TopicRegistry
    participant LM as LogManager
    participant PL as PluginLoader
    participant UP as IUploadPlugin
    participant SAAS as SaaS API

    MW->>AC: start_upload()
    AC->>TR: get_topic_meta("HR")
    TR-->>AC: TopicMeta{schema_path, upload_endpoint, upload_plugin: "hr_upload"}

    Note over AC: Spawn background thread

    AC->>PL: load("./plugins/upload/hr_upload.so")
    PL-->>AC: IUploadPlugin*

    AC->>UP: initialize(config)
    Note over AC,UP: config includes: full config tree, topic, options, proxy, endpoint

    AC->>UP: upload(m_current_data)

    UP->>UP: ensure_authenticated()

    alt Need new token
        UP->>SAAS: POST /api/refreshtoken {user:{LoginName, Loginpassword}}
        SAAS-->>UP: {Token: "refresh_token"}
        UP->>SAAS: GET /api/token/ [Bearer: refresh_token]
        SAAS-->>UP: {AccessToken, AccessTokenExpiryDateTime}
    else Token valid
        Note over UP: Reuse cached m_access_token
    end

    UP->>SAAS: POST <upload_endpoint> [Bearer: access_token] {data, options}
    SAAS-->>UP: 200 OK

    UP-->>AC: expected<void, string>

    AC->>LM: get_audit_logger("HR")
    LM->>LM: Log with Ed25519 signature
    Note over LM: "SUCCESS: Upload of topic HR completed. OS User: ... | Computer: ..."

    AC-->>MW: on_upload_complete() + update_log()
```

### 4.1 Payload Merging (Topic-Specific Options)

```cpp
// In AppController::start_upload() background thread:
nlohmann::json plugin_config = config;  // full config tree
plugin_config["topic"] = m_current_topic;
plugin_config["upload_endpoint"] = meta->upload_endpoint;
plugin_config["options"]["dateFormat"] = m_date_format;

// Merge topic-specific overrides
if (config["topics"][m_current_topic]["options"].is_object()) {
    for (auto& [key, value] : config["topics"][m_current_topic]["options"].items()) {
        plugin_config["options"][key] = value;
    }
}
```

This allows per-topic settings like:
```ini
[topics.HR.options]
updateExistingRecords = "true"
insertBaseTables = "false"
autoCreatePortalUser = "false"
dateFormat = "dd.mm.yyyy"
```

---

## 5. Plugin Metadata Discovery Flow

Used for the "Manage Plugins" dialog.

```mermaid
flowchart TB
    subgraph Discovery Process
        A["get_all_plugins()"]
        B[scan_dir: plugins/data]
        C[scan_dir: plugins/upload]

        D[For each .so/.dll file]
        E["PluginLoader::load(file)"]
        F{Loaded OK?}
        G{"Plugin type matches directory?"}

        H[Extract metadata]
        I["Collect PluginInfo"]
        J["plugin.shutdown()"]
        K["Return vector<PluginInfo>"]
    end

    A --> B --> D
    A --> C --> D
    D --> E
    E --> F
    F -->|Yes| G
    F -->|No| D
    G -->|Yes| H --> I
    G -->|No| D
    I --> J --> D
    D -->|All files done| K
```
