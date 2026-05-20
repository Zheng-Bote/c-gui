# Technical Documentation — UML Class Design

---

## 1. Plugin Interface Hierarchy

The plugin system is the central extensibility mechanism. All plugins implement the pure virtual `IPlugin` base interface and are further specialized into `IDataPlugin` and `IUploadPlugin`.

```mermaid
classDiagram
    class IPlugin {
        <<interface>>
        +initialize(json config) bool
        +get_topic() string
        +get_version() string
        +get_type() PluginType
        +shutdown() void
    }

    class IDataPlugin {
        <<interface>>
        +fetch_batch(size_t max_records) vector~json~
        +get_interface_type() string
        +get_type() PluginType
    }

    class IUploadPlugin {
        <<interface>>
        +upload(json data) expected~void, string~
        +get_type() PluginType
    }

    class PluginType {
        <<enumeration>>
        DATA
        UPLOAD
    }

    IPlugin <|-- IDataPlugin : extends
    IPlugin <|-- IUploadPlugin : extends
    IDataPlugin --> PluginType : returns
    IUploadPlugin --> PluginType : returns

    class PluginLoader {
        -m_handle : void*
        -m_plugin : IPlugin*
        -m_destroy_func : DestroyPluginFunc
        +load(path) expected~void, string~
        +unload() void
        +get_plugin() IPlugin*
    }

    PluginLoader --> IPlugin : owns

    class CsvInputPlugin {
        -m_file_path : string
        -m_headers : vector~string~
        -m_file : ifstream
        +initialize() bool
        +fetch_batch() vector~json~
        +get_interface_type() string
    }

    class HrJsonInputPlugin {
        -m_file_path : string
        -m_data : json
        -m_current_pos : size_t
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class XlsxInputPlugin {
        -m_file_path : string
        -m_doc : unique_ptr~XLDocument~
        -m_current_row : uint32_t
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class DbInputPlugin {
        -m_connection_string : string
        -m_connection : unique_ptr~pqxx::connection~
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class HrDbOraInputPlugin {
        -m_connection_string : string
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class KafkaInputPlugin {
        -m_initialized : bool
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class HealthSqliteInputPlugin {
        -m_db_path : string
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class DeptOraInputPlugin {
        -m_connection_string : string
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class LocS3InputPlugin {
        -m_s3_bucket : string
        +initialize() bool
        +fetch_batch() vector~json~
    }

    class HrUploadPlugin {
        -m_config : json
        -m_base_url : string
        -m_refresh_token : string
        -m_access_token : string
        -m_access_expiry : time_point
        -m_proxy : string
        +initialize() bool
        +upload() expected~void, string~
        -ensure_authenticated() expected~void, string~
        -perform_refresh() expected~void, string~
        -fetch_access_token() expected~void, string~
    }

    IDataPlugin <|-- CsvInputPlugin : implements
    IDataPlugin <|-- HrJsonInputPlugin : implements
    IDataPlugin <|-- XlsxInputPlugin : implements
    IDataPlugin <|-- DbInputPlugin : implements
    IDataPlugin <|-- HrDbOraInputPlugin : implements
    IDataPlugin <|-- KafkaInputPlugin : implements
    IDataPlugin <|-- HealthSqliteInputPlugin : implements
    IDataPlugin <|-- DeptOraInputPlugin : implements
    IDataPlugin <|-- LocS3InputPlugin : implements
    IUploadPlugin <|-- HrUploadPlugin : implements
```

---

## 2. Core Library Classes

```mermaid
classDiagram
    class AppController {
        -m_config_manager : unique_ptr~ConfigManager~
        -m_plugin_loader : unique_ptr~PluginLoader~
        -m_topic_registry : unique_ptr~TopicRegistry~
        -m_schema_manager : unique_ptr~SchemaManager~
        -m_validator : unique_ptr~Validator~
        -m_uploader : unique_ptr~Uploader~
        -m_window : MainWindow*
        -m_current_topic : string
        -m_date_format : string
        -m_current_data : json
        -m_preview_data : json
        +init(ini_path, password) expected~void, string~
        +get_topics() vector~string~
        +select_topic(topic) void
        +set_date_format(format) void
        +get_effective_date_format(topic) string
        +get_available_interfaces(topic) vector~string~
        +load_data(interface, path_or_conn) expected~void, string~
        +get_default_data_source(topic, interface) string
        +get_all_plugins() vector~PluginInfo~
        +start_validation() void
        +start_upload() void
        -update_log(message) void
        -update_progress(progress) void
        -on_validation_complete(success, errors) void
        -on_upload_complete(result) void
    }

    class ConfigManager {
        -m_config : json
        +load_encrypted_ini(path, password) expected~void, string~
        +get_config() const json&
        -secure_clear(data) void
    }

    class TopicRegistry {
        -m_topics : map~string, TopicMeta~
        +register_topic(meta) void
        +get_topic_meta(topic) optional~TopicMeta~
        +get_all_topics() vector~string~
    }

    class TopicMeta {
        +topic : string
        +schema_path : path
        +upload_endpoint : string
        +upload_plugin : string
        +proxy : string
        +upload_timeout : long
    }

    class SchemaManager {
        -m_schemas : map~path, shared_ptr~Schema~~
        +get_schema(path) shared_ptr~Schema~
    }

    class Validator {
        <<static>>
        +validate(doc, schema, errors) bool
    }

    class ValidationError {
        +path : string
        +message : string
    }

    class AuthManager {
        <<static>>
        +authenticate(base_url, login, password) expected~AuthTokens, string~
        +refresh_access_token(base_url, refresh_token) expected~AuthTokens, string~
    }

    class AuthTokens {
        +access_token : string
        +refresh_token : string
        +access_expiry : string
    }

    class Uploader {
        -m_verify_ssl : bool
        -m_proxy : string
        -m_timeout : long
        +upload_sync(url, payload, bearer_token) expected~void, string~
        +upload_async(url, payload, callback, bearer_token) void
        +configure_ssl(verify, ca_path) void
        +set_proxy(proxy) void
        +set_timeout(timeout) void
    }

    class PluginInfo {
        +name : string
        +topic : string
        +type : string
        +version : string
        +interface_type : string
        +path : path
    }

    AppController *-- ConfigManager : owns
    AppController *-- PluginLoader : owns
    AppController *-- TopicRegistry : owns
    AppController *-- SchemaManager : owns
    AppController *-- Validator : owns
    AppController *-- Uploader : owns
    AppController --> MainWindow : observes
    AppController --> PluginInfo : returns
    TopicRegistry --> TopicMeta : stores
    Validator --> ValidationError : returns
    AuthManager --> AuthTokens : returns
```

---

## 3. LogManager Architecture

```mermaid
classDiagram
    class LogManager {
        <<singleton>>
        -m_log_path : string
        -m_level : level_enum
        -m_signing_key : vector~unsigned char~
        -m_loggers : map~string, shared_ptr~logger~~
        -m_audit_loggers : map~string, shared_ptr~logger~~
        +get_instance() LogManager&
        +initialize(log_path, log_level) void
        +set_callback(callback) void
        +get_logger(topic) shared_ptr~logger~
        +get_audit_logger(topic) shared_ptr~logger~
        +get_core_logger() shared_ptr~logger~
        +set_signing_key(hex_key) void
    }

    class audit_signing_sink {
        -m_target_sink : shared_ptr~sink~
        -m_private_key : vector~unsigned char~
        #sink_it_(msg) void
        +Ed25519 sign each log entry
        +append sig: <hex> to entry
    }

    LogManager --> audit_signing_sink : creates for audit loggers

    class spdlog_logger {
        <<spdlog::logger>>
        +info(...)
        +error(...)
        +warn(...)
        +debug(...)
    }

    class rotating_file_sink_mt {
        <<spdlog::sinks::rotating_file_sink_mt>>
        -filename : string
        -max_size : size_t
        -max_files : int
    }

    class stdout_color_sink_mt {
        <<spdlog::sinks::stdout_color_sink_mt>>
    }

    class callback_sink_mt {
        <<spdlog::sinks::callback_sink_mt>>
        +callback(msg)
    }

    LogManager --> spdlog_logger : manages
    spdlog_logger --> rotating_file_sink_mt : has
    spdlog_logger --> stdout_color_sink_mt : has
    spdlog_logger --> callback_sink_mt : has
    audit_signing_sink --> rotating_file_sink_mt : wraps
```

---

## 4. MainWindow UI Class

```mermaid
classDiagram
    class MainWindow {
        -m_controller : AppController*
        -m_topic_choice : wxComboBox*
        -m_interface_choice : wxChoice*
        -m_date_format_choice : wxChoice*
        -m_date_delimiter_choice : wxChoice*
        -m_load_btn : wxButton*
        -m_validate_btn : wxButton*
        -m_upload_btn : wxButton*
        -m_data_list : wxListCtrl*
        -m_log_ctrl : wxTextCtrl*
        -m_progress_gauge : wxGauge*
        +MainWindow(title, controller)
        +update_log(message) void
        +update_progress(progress) void
        +update_preview() void
        +clear_preview() void
        +update_buttons(can_validate, can_upload) void
        -on_exit(event) void
        -on_about(event) void
        -on_manage_plugins(event) void
        -on_topic_selected(event) void
        -on_load_data(event) void
        -on_validate(event) void
        -on_upload(event) void
        -on_date_format_changed(event) void
    }

    class CGuiApp {
        -m_controller : unique_ptr~AppController~
        +OnInit() bool
    }

    class wxFrame {
        <<wxWidgets>>
    }

    class wxApp {
        <<wxWidgets>>
    }

    wxApp <|-- CGuiApp : extends
    wxFrame <|-- MainWindow : extends
    MainWindow --> AppController : collaborates
    CGuiApp --> AppController : owns
    CGuiApp --> MainWindow : creates
```