# HR JSON Input Plugin

This plugin provides JSON data ingestion capabilities for the **HR** topic in the `c-gui` application.

## Overview

- **Topic:** HR
- **Interface Type:** `json`
- **Format:** A JSON file containing an array of objects.

## Features

- Reads records from a local JSON file.
- Supports arbitrary object structures as long as they represent individual records.
- Efficiently loads the entire array into the application memory.

## Configuration

When initialized by the `AppController`, the plugin expects a JSON configuration containing:

- `topic`: Must be "HR".
- `file_path`: Absolute or relative path to the `.json` file.

## Data Format Requirements (Schema v1)

The JSON file must contain a top-level array of objects. Each object should follow the HR schema. Required fields are marked with `*`.

- `EmployeeNumber`* (6-8 digits)
- `FirstName`*
- `LastName`*
- `SSNValue`
- `DateOfBirth` (Format: `dd.mm.yyyy`)
- `Email`
- `Organization.Code`
- `Department.Code`

### Example

```json
[
  {
    "EmployeeNumber": "123456",
    "FirstName": "John",
    "LastName": "Doe",
    "DateOfBirth": "15.05.1985",
    "Email": "john.doe@example.com",
    "Organization.Code": "ORG01",
    "Department.Code": "DEPT_IT"
  },
  {
    "EmployeeNumber": "234567",
    "FirstName": "Jane",
    "LastName": "Smith",
    "DateOfBirth": "20.11.1990",
    "Email": "jane.smith@example.com",
    "Organization.Code": "ORG01",
    "Department.Code": "DEPT_HR"
  }
]
```
