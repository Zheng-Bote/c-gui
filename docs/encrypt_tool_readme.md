# Encrypt Tool

The `encrypt_tool` is a small CLI utility used to encrypt your `sample.ini` (or any `.ini`) configuration files into a secure `.enc` format. This prevents sensitive data like API keys, SaaS logins, and database credentials from being stored in plain text.

## Overview

The tool uses **libsodium** (XChaCha20-Poly1305) for encryption and **Argon2id** for key derivation from your password.

## Usage

```bash
encrypt_tool <input_ini> <output_enc> [<password>]
```

### Password Provisioning

There are two ways to provide the encryption password:

1.  **Environment Variable (Recommended for Automation):** Set the `cgui_config` variable.
2.  **Command-line Parameter:** Pass the password as the third argument.

> **Note:** A password provided via the command-line parameter will always take precedence over the environment variable.

---

## Operating System Examples

### Windows (PowerShell)

#### Using Environment Variable
```powershell
# Set the variable for the current session
$env:cgui_config = "YourSecurePassword123!"

# Run the tool without the password parameter
.\bin\encrypt_tool.exe sample.ini config.enc
```

#### Using Command-line Parameter
```powershell
.\bin\encrypt_tool.exe sample.ini config.enc "YourSecurePassword123!"
```

---

### Linux (Bash / Zsh)

#### Using Environment Variable
```bash
# Set the variable and run the tool
export cgui_config="YourSecurePassword123!"
./bin/encrypt_tool sample.ini config.enc

# OR: Set it inline for a single command
cgui_config="YourSecurePassword123!" ./bin/encrypt_tool sample.ini config.enc
```

#### Using Command-line Parameter
```bash
./bin/encrypt_tool sample.ini config.enc "YourSecurePassword123!"
```

## Security Best Practices

- **Clear History:** If you provide the password via command-line parameters, it may be stored in your shell history. Use environment variables or clear your history afterwards.
- **Secure Storage:** The `.enc` file can be safely stored on disk, but ensure your password is kept in a secure location (e.g., a Password Manager).
- **Automation:** When using the tool in CI/CD pipelines, always use secret-masked environment variables.
