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
    
    // Support hierarchical sections like [topics.HR]
    nlohmann::json* current = config;
    size_t pos = 0;
    while ((pos = s.find('.')) != std::string::npos) {
        std::string part = s.substr(0, pos);
        current = &((*current)[part]);
        s.erase(0, pos + 1);
    }
    (*current)[s][name] = value;
    
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
    if (ini_parse_string(ini_content.c_str(), ini_handler, &m_config) != 0) {
        secure_clear(ini_content);
        logger->error("Failed to parse decrypted INI content");
        return std::unexpected("Failed to parse decrypted INI content");
    }

    secure_clear(ini_content);
    logger->info("Config file decrypted and parsed successfully: {}", path.string());
    return {};
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
