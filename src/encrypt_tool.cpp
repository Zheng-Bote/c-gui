/**
 * SPDX-FileComment: Encryption Tool for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file encrypt_tool.cpp
 * @brief CLI tool to encrypt INI configuration files.
 * @version 1.0.1
 * @date 2026-05-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <sodium.h>
#include <string>
#include <cstdlib>

/**
 * @brief Securely clear sensitive data.
 * @param data Vector to clear.
 */
void secure_clear(std::vector<unsigned char>& data) {
    sodium_memzero(data.data(), data.size());
}

int main(int argc, char* argv[]) {
    std::string password;
    char* env_pwd = std::getenv("cgui_config");
    if (env_pwd != nullptr) {
        password = env_pwd;
    }

    if (argc < 3 || (password.empty() && argc < 4)) {
        std::cerr << "Usage: " << argv[0] << " <input_ini> <output_enc> [<password>]" << std::endl;
        std::cerr << "Note: Password can also be set via the environment variable 'cgui_config'." << std::endl;
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path = argv[2];
    
    // Command line argument takes precedence over environment variable
    if (argc >= 4) {
        password = argv[3];
    }

    if (sodium_init() < 0) {
        std::cerr << "Libsodium initialization failed" << std::endl;
        return 1;
    }

    std::ifstream input_file(input_path, std::ios::binary | std::ios::ate);
    if (!input_file.is_open()) {
        std::cerr << "Could not open input file: " << input_path << std::endl;
        return 1;
    }

    std::streamsize size = input_file.tellg();
    input_file.seekg(0, std::ios::beg);
    std::vector<unsigned char> plaintext(size);
    if (!input_file.read(reinterpret_cast<char*>(plaintext.data()), size)) {
        std::cerr << "Failed to read input file" << std::endl;
        return 1;
    }

    std::vector<unsigned char> salt(crypto_pwhash_SALTBYTES);
    randombytes_buf(salt.data(), salt.size());

    std::vector<unsigned char> key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    if (crypto_pwhash(key.data(), key.size(), password.c_str(), password.length(), salt.data(),
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        std::cerr << "Key derivation failed" << std::endl;
        secure_clear(key);
        return 1;
    }

    std::vector<unsigned char> nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    randombytes_buf(nonce.data(), nonce.size());

    std::vector<unsigned char> ciphertext(plaintext.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long ciphertext_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(ciphertext.data(), &ciphertext_len,
                                               plaintext.data(), plaintext.size(),
                                               nullptr, 0,
                                               nullptr, nonce.data(), key.data());

    secure_clear(key);

    std::ofstream output_file(output_path, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Could not open output file: " << output_path << std::endl;
        return 1;
    }

    output_file.write(reinterpret_cast<char*>(salt.data()), salt.size());
    output_file.write(reinterpret_cast<char*>(nonce.data()), nonce.size());
    output_file.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext_len);

    std::cout << "Successfully encrypted " << input_path << " to " << output_path << std::endl;

    return 0;
}
