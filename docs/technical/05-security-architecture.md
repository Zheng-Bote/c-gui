# Technical Documentation — Security Architecture

---

## 1. Core Security Principles

**c-gui** is built on a "Secure by Default" philosophy, focusing on protecting sensitive configuration data, ensuring log integrity, and providing safe execution environments.

---

## 2. Configuration Protection

### 2.1 Encryption Standard
Configuration files (`.enc`) are protected using:
- **Algorithm**: XChaCha20-Poly1305 (IETF variant)
- **Key Derivation**: Argon2id (`crypto_pwhash_ALG_ARGON2ID13`)
- **Key Size**: 256-bit (32 bytes)
- **Nonce Size**: 192-bit (24 bytes)

### 2.2 Password Verification
Every configuration change in the GUI requires re-entering the configuration password. The `ConfigManager` verifies the password by attempting a full file decryption before applying any updates, preventing accidental data corruption or unauthorized changes.

---

## 3. Audit Log Integrity

### 3.1 Ed25519 Digital Signatures
Critical actions (e.g., Data Upload) are logged in a structured format and cryptographically signed.
- **Algorithm**: Ed25519 (`crypto_sign_detached`)
- **Persistence**: 64-byte hex-encoded signatures are appended to each log entry.
- **Verification**: Any modification to the log entry will invalidate the signature, ensuring non-repudiation.

### 3.2 Credential Masking
The application automatically redacts sensitive keys (e.g., `password`, `api_secret`, `api_key`) from:
- Persistent log files (`logs/*.log`)
- GUI status updates
- Console output

---

## 4. Execution Sandbox (WASM)

### 4.1 Hybrid Isolation
While native plugins have full system access, **WebAssembly (WASM)** plugins run in a strictly isolated sandbox provided by the **Wasmtime** engine.

### 4.2 Restricted Networking
WASM plugins do not have direct access to host sockets. All network communication is performed via a **Host-Assisted Bridge**:
- **Control**: The host application (`libcurl`) manages the actual connection.
- **Security**: The host enforces global proxy settings and SSL validation using the **Native System CA Store**.
- **Transparency**: Every HTTP request made by a WASM plugin is logged by the core application.

---

## 5. Memory Security

- **Secure Erasure**: Sensitive data (keys, passwords, decrypted plaintext) is wiped from memory using `sodium_memzero` as soon as it is no longer needed.
- **Static Linking**: The core security logic is statically linked into `cgui_core` to prevent DLL hijacking of cryptographic functions.
