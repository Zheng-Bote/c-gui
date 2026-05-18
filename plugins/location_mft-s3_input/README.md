# Location S3 Input Plugin

This plugin provides Amazon S3 Managed File Transfer (MFT) ingestion capabilities for the **Location** topic.

## Overview

- **Topic:** Location
- **Interface Type:** `mft-s3`
- **Format:** Files (e.g., JSON or CSV) retrieved from an S3 bucket.

## Configuration

When initialized by the `AppController`, the plugin expects:

- `topic`: Must be "Location".
- `connection_string`: S3 Bucket name or access details.

## Implementation Details

This is a **stub** implementation. In a production environment, this plugin would utilize the `aws-sdk-cpp` to download files from an S3 bucket and parse them into JSON records.
