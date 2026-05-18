# HR CSV Input Plugin

This plugin provides CSV (Comma Separated Values) data ingestion capabilities for the **HR** topic in the `c-gui` application.

## Overview

- **Topic:** HR
- **Interface Type:** `csv`
- **Format:** Standard CSV with headers in the first row.

## Features

- Reads records from a local CSV file.
- Automatically maps the first row as field names (headers).
- Simple comma-delimited parsing.

## Configuration

When initialized by the `AppController`, the plugin expects a JSON configuration containing:

- `topic`: Must be "HR".
- `file_path`: Absolute or relative path to the `.csv` file.

## Data Format Requirements (Schema v1)

The CSV file should include the following fields as headers. Fields marked with `*` are required.

- `EmployeeNumber`* (6-8 digits)
- `FirstName`*
- `LastName`*
- `SSNValue`
- `DateOfBirth` (Format: `dd.mm.yyyy`)
- `Email`
- `Organization.Code`
- `Department.Code`

### Example

```csv
EmployeeNumber,FirstName,LastName,DateOfBirth,Email,Organization.Code,Department.Code
123456,John,Doe,15.05.1985,john.doe@example.com,ORG01,DEPT_IT
234567,Jane,Smith,20.11.1990,jane.smith@example.com,ORG01,DEPT_HR
```

*Note: This basic implementation currently expects commas as delimiters and does not support advanced quoting or escaping.*
