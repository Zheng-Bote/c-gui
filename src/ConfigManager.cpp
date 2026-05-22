/**
 * SPDX-FileComment: ConfigManager implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file ConfigManager.cpp
 * @brief Implementation of encrypted INI configuration loading.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "ConfigManager.hpp"
#include "LogManager.hpp"
#include <sodium.h>
#include <fstream>
#include <ini.h>
#include <iostream>

namespace cgui {

ConfigManager::~ConfigManager() {
    // Clear config on destruction
    m_config.clear();
}

/**
 * @brief Callback for inih parser.
 */
static int ini_handler(void* user, const char* section, const char* name, const char* value) {
    auto* config = static_cast<nlohmann::json*>(user);
    std::string s(section);
    
    std::string val(value);
    // Strip leading and trailing quotes if they exist
    if (val.length() >= 2 && val.front() == '"' && val.back() == '"') {
        val = val.substr(1, val.length() - 2);
    }
    
    // Support hierarchical sections like [topics.HR]
    nlohmann::json* current = config;
    size_t pos = 0;
    while ((pos = s.find('.')) != std::string::npos) {
        std::string part = s.substr(0, pos);
        current = &((*current)[part]);
        s.erase(0, pos + 1);
    }
    (*current)[s][name] = val;
    
    return 1;
}

std::expected<void, std::string> ConfigManager::load_encrypted_ini(const std::filesystem::path& path, const std::string& password) {
    auto logger = LogManager::get_instance().get_core_logger();
    logger->info("Starting decryption of config file: {}", path.string());

    if (sodium_init() < 0) {
        logger->error("Libsodium initialization failed");
        return std::unexpected("Libsodium initialization failed");
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        logger->error("Could not open file: {}", path.string());
        return std::unexpected("Could not open file: " + path.string());
    }

    std::streamsize size = file.tellg();
    if (size < crypto_pwhash_SALTBYTES + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + crypto_aead_xchacha20poly1305_ietf_ABYTES) {
        logger->error("File too small to be a valid encrypted config: {}", path.string());
        return std::unexpected("File too small to be a valid encrypted config");
    }

    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> salt(crypto_pwhash_SALTBYTES);
    std::vector<unsigned char> nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    
    if (!file.read(reinterpret_cast<char*>(salt.data()), salt.size())) {
        logger->error("Failed to read salt from config file");
        return std::unexpected("Failed to read salt");
    }
    if (!file.read(reinterpret_cast<char*>(nonce.data()), nonce.size())) {
        logger->error("Failed to read nonce from config file");
        return std::unexpected("Failed to read nonce");
    }

    std::vector<unsigned char> ciphertext(size - crypto_pwhash_SALTBYTES - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    if (!file.read(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size())) {
        logger->error("Failed to read ciphertext from config file");
        return std::unexpected("Failed to read ciphertext");
    }

    // Derive key
    logger->debug("Deriving key from password...");
    std::vector<unsigned char> key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    if (crypto_pwhash(key.data(), key.size(), password.c_str(), password.length(), salt.data(),
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        secure_clear(key);
        logger->error("Key derivation failed (out of memory?)");
        return std::unexpected("Key derivation failed (out of memory?)");
    }

    // Decrypt
    logger->debug("Decrypting content...");
    std::vector<unsigned char> decrypted(ciphertext.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long decrypted_len;
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(decrypted.data(), &decrypted_len,
                                                   nullptr,
                                                   ciphertext.data(), ciphertext.size(),
                                                   nullptr, 0,
                                                   nonce.data(), key.data()) != 0) {
        secure_clear(key);
        secure_clear(decrypted);
        logger->error("Decryption failed. Incorrect password?");
        return std::unexpected("Decryption failed. Incorrect password?");
    }

    secure_clear(key);

    std::string ini_content(reinterpret_cast<char*>(decrypted.data()), decrypted_len);
    secure_clear(decrypted);

    // Parse INI
    logger->debug("Parsing decrypted INI content...");
    m_config.clear();

    auto custom_ini_parse = [](const std::string& content, int (*handler)(void*, const char*, const char*, const char*), void* user) -> int {
        std::stringstream ss(content);
        std::string line;
        std::string current_section;
        int line_num = 0;

        while (std::getline(ss, line)) {
            line_num++;
            if (!line.empty() && line.back() == '\r') line.pop_back();

            size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos) continue; 
            
            std::string trimmed = line.substr(first);
            if (trimmed[0] == ';' || trimmed[0] == '#') continue; 

            if (trimmed[0] == '[' ) {
                size_t end = trimmed.find(']');
                if (end != std::string::npos) {
                    current_section = trimmed.substr(1, end - 1);
                    continue;
                }
            }

            size_t eq_pos = trimmed.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = trimmed.substr(0, eq_pos);
                std::string val = trimmed.substr(eq_pos + 1);

                key.erase(key.find_last_not_of(" \t") + 1);
                size_t val_first = val.find_first_not_of(" \t");
                if (val_first != std::string::npos) {
                    val = val.substr(val_first);
                    size_t val_last = val.find_last_not_of(" \t");
                    val = val.substr(0, val_last + 1);
                } else {
                    val = "";
                }

                if (handler(user, current_section.c_str(), key.c_str(), val.c_str()) == 0) return line_num;
            }
        }
        return 0;
    };

    if (custom_ini_parse(ini_content, ini_handler, &m_config) != 0) {
        secure_clear(ini_content);
        logger->error("Failed to parse decrypted INI content");
        return std::unexpected("Failed to parse decrypted INI content");
    }

    secure_clear(ini_content);
    logger->info("Config file decrypted and parsed successfully: {}", path.string());
    return {};
}

std::expected<void, std::string> ConfigManager::verify_password(const std::filesystem::path& path, const std::string& password) {
    if (!std::filesystem::exists(path)) return {}; // Nothing to verify against

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return std::unexpected("Could not open file for verification");

    std::vector<unsigned char> salt(crypto_pwhash_SALTBYTES);
    std::vector<unsigned char> nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    
    if (!file.read(reinterpret_cast<char*>(salt.data()), salt.size()) ||
        !file.read(reinterpret_cast<char*>(nonce.data()), nonce.size())) {
        return std::unexpected("File corrupted or too small");
    }

    std::vector<unsigned char> key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    if (crypto_pwhash(key.data(), key.size(), password.c_str(), password.length(), salt.data(),
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        secure_clear(key);
        return std::unexpected("Key derivation failed");
    }

    // Attempt to decrypt the whole file
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(crypto_pwhash_SALTBYTES + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES, std::ios::beg);

    std::vector<unsigned char> ciphertext(size - crypto_pwhash_SALTBYTES - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    file.read(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size());

    std::vector<unsigned char> decrypted(ciphertext.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long decrypted_len;
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(decrypted.data(), &decrypted_len, nullptr,
                                                   ciphertext.data(), ciphertext.size(),
                                                   nullptr, 0, nonce.data(), key.data()) != 0) {
        secure_clear(key);
        secure_clear(decrypted);
        return std::unexpected("Incorrect password");
    }

    secure_clear(key);
    secure_clear(decrypted);
    return {};
}

static void json_to_ini_recursive(const nlohmann::json& j, const std::string& section, std::string& out) {
    bool has_values = false;
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (!it.value().is_object()) {
            has_values = true;
            break;
        }
    }

    // Force header for networking to allow commented proxy
    bool force_header = (section == "networking");

    if (has_values || force_header) {
        if (!section.empty()) {
            out += "[" + section + "]\n";
        }
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (!it.value().is_object()) {
                std::string val;
                if (it.value().is_string()) {
                    std::string s_val = it.value().get<std::string>();
                    // Quote if contains special characters
                    if (s_val.find(' ') != std::string::npos || 
                        s_val.find(',') != std::string::npos || 
                        s_val.find(';') != std::string::npos || 
                        s_val.find('#') != std::string::npos || 
                        (s_val.length() > 0 && s_val[0] == '"')) {
                        val = "\"" + s_val + "\"";
                    } else {
                        val = s_val;
                    }
                } else {
                    val = it.value().dump();
                }
                out += it.key() + "=" + val + "\n";
            }
        }
        
        // Requirement: If proxy is missing in networking, write it commented out
        if (section == "networking" && !j.contains("proxy")) {
            out += "; proxy=\"myuser:mypass@proxy.example.com:8080\"\n";
        }
        
        out += "\n";
    }

    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it.value().is_object()) {
            std::string next_section = section.empty() ? it.key() : section + "." + it.key();
            json_to_ini_recursive(it.value(), next_section, out);
        }
    }
}

std::expected<void, std::string> ConfigManager::save_encrypted_ini(const std::filesystem::path& path, const std::string& password) {
    auto logger = LogManager::get_instance().get_core_logger();
    logger->info("Starting encryption of config file: {}", path.string());

    if (sodium_init() < 0) {
        return std::unexpected("Libsodium initialization failed");
    }

    std::string ini_content;
    json_to_ini_recursive(m_config, "", ini_content);

    std::vector<unsigned char> salt(crypto_pwhash_SALTBYTES);
    randombytes_buf(salt.data(), salt.size());

    std::vector<unsigned char> key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    if (crypto_pwhash(key.data(), key.size(), password.c_str(), password.length(), salt.data(),
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        secure_clear(key);
        return std::unexpected("Key derivation failed");
    }

    std::vector<unsigned char> nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    randombytes_buf(nonce.data(), nonce.size());

    std::vector<unsigned char> ciphertext(ini_content.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long ciphertext_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(ciphertext.data(), &ciphertext_len,
                                               reinterpret_cast<const unsigned char*>(ini_content.data()), ini_content.size(),
                                               nullptr, 0,
                                               nullptr, nonce.data(), key.data());

    secure_clear(key);

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::unexpected("Could not open file for writing: " + path.string());
    }

    file.write(reinterpret_cast<char*>(salt.data()), salt.size());
    file.write(reinterpret_cast<char*>(nonce.data()), nonce.size());
    file.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext_len);

    logger->info("Config file saved and encrypted successfully: {}", path.string());
    return {};
}

void ConfigManager::update_config(const std::string& section, const std::string& key, const std::string& value) {
    nlohmann::json* current = &m_config;
    std::string s = section;
    size_t pos = 0;
    while ((pos = s.find('.')) != std::string::npos) {
        std::string part = s.substr(0, pos);
        current = &((*current)[part]);
        s.erase(0, pos + 1);
    }
    
    // Requirement: If proxy is empty, remove the key so it gets commented out in INI
    if (value.empty() && section == "networking" && key == "proxy") {
        if ((*current).contains(s) && (*current)[s].contains(key)) {
            (*current)[s].erase(key);
        }
    } else {
        (*current)[s][key] = value;
    }
}

const nlohmann::json& ConfigManager::get_config() const {
    return m_config;
}

void ConfigManager::secure_clear(std::string& data) {
    sodium_memzero(data.data(), data.size());
}

void ConfigManager::secure_clear(std::vector<unsigned char>& data) {
    sodium_memzero(data.data(), data.size());
}

} // namespace cgui
