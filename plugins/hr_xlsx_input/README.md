# HR XLSX Input Plugin

This plugin provides Microsoft Excel (.xlsx) data ingestion capabilities for the **HR** topic in the `c-gui` application, powered by the `OpenXLSX` library.

## Overview

- **Topic:** HR
- **Interface Type:** `xlsx`
- **Format:** Excel OpenXML Spreadsheet (.xlsx).

## Features

- **Type Safety:** Automatically detects Excel cell types (String, Integer, Float, Boolean) and preserves them in the resulting JSON.
- **Header Detection:** Uses the first row of the first worksheet as the field names.
- **Null Handling:** Correctly identifies empty cells and maps them to JSON `null`.
- **Row Management:** Skips rows that contain no data.

## Configuration

When initialized by the `AppController`, the plugin expects a JSON configuration containing:

- `topic`: Must be "HR".
- `file_path`: Absolute or relative path to the `.xlsx` file.

## Data Requirements (Schema v1)

- Uses the first worksheet found in the workbook.
- Row 1 must contain the headers (field names).
- Data must start on Row 2.

### Required Fields (*)

- `EmployeeNumber`* (6-8 digits)
- `FirstName`*
- `LastName`*

### Optional Fields (Example)

- `SSNValue`
- `DateOfBirth` (Format: `dd.mm.yyyy`)
- `Email`
- `Organization.Code`
- `Department.Code`

## Dependencies

- **OpenXLSX:** Required for parsing the `.xlsx` format.
