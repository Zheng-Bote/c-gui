# HR Oracle Input Plugin

This plugin allows ingestion of data from an Oracle database using the SOCI library.

## Prerequisites

1.  **Oracle Instant Client**:
    You must have the Oracle Instant Client (v19 or newer) installed on your system.
    
    Current local path: `C:\Users\zb_ba\Downloads\Zip\instantclient-basic-windows.x64-23.26.1.0.0\instantclient_23_0`

2.  **System PATH**:
    Add the above path to your Windows `PATH` environment variable so that the application can find `oci.dll` at runtime.

3.  **Oracle SDK (for building)**:
    If you are building the project from source and SOCI needs to be recompiled, ensure the `sdk` folder is present within the Instant Client directory, or set the `ORACLE_HOME` environment variable to point to the client directory.

## Configuration

In your `config.ini` (or the encrypted `config.enc` via GUI):

```ini
[topics.HR]
data_db-ora = "host:your-server, port:1521, user:username, password:password, service:xe"
```

The plugin will use this string to establish a connection via the Oracle EZConnect or TNS format.
