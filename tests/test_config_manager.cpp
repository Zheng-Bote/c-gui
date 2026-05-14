/**
 * SPDX-FileComment: Unit tests for ConfigManager
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file tests/test_config_manager.cpp
 * @brief Unit tests for the ConfigManager class
 * @version 1.0.0
 * @date 2026-05-14
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>
#include "ConfigManager.hpp"
#include <sodium.h>
#include <fstream>
#include <filesystem>

using namespace cgui;

namespace {
void create_encrypted_ini(const std::filesystem::path& path, const std::string& ini_content, const std::string& password) {
    if (sodium_init() < 0) throw std::runtime_error("sodium_init failed");

    std::vector<unsigned char> salt(crypto_pwhash_SALTBYTES);
    randombytes_buf(salt.data(), salt.size());

    std::vector<unsigned char> key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    REQUIRE(crypto_pwhash(key.data(), key.size(), password.c_str(), password.length(), salt.data(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                          crypto_pwhash_ALG_ARGON2ID13) == 0);

    std::vector<unsigned char> nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    randombytes_buf(nonce.data(), nonce.size());

    std::vector<unsigned char> ciphertext(ini_content.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long ciphertext_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(ciphertext.data(), &ciphertext_len,
                                               reinterpret_cast<const unsigned char*>(ini_content.data()), ini_content.size(),
                                               nullptr, 0, nullptr, nonce.data(), key.data());

    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<char*>(salt.data()), salt.size());
    file.write(reinterpret_cast<char*>(nonce.data()), nonce.size());
    file.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext_len);
}
}

TEST_CASE("ConfigManager loads and decrypts config", "[ConfigManager]") {
    ConfigManager manager;
    std::filesystem::path test_path = "test_config.enc";
    std::string password = "test_password";
    std::string ini_content = "[Section]\nkey=value\n";

    create_encrypted_ini(test_path, ini_content, password);

    auto result = manager.load_encrypted_ini(test_path, password);
    REQUIRE(result.has_value());

    const auto& config = manager.get_config();
    REQUIRE(config.contains("Section"));
    REQUIRE(config["Section"]["key"] == "value");

    SECTION("Fails with wrong password") {
        auto fail_result = manager.load_encrypted_ini(test_path, "wrong_password");
        REQUIRE(!fail_result.has_value());
    }

    std::filesystem::remove(test_path);
}
