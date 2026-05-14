/**
 * SPDX-FileComment: ConfigManager for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file ConfigManager.hpp
 * @brief Handles encrypted INI configuration loading.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_CONFIG_MANAGER_HPP
#define C_GUI_CONFIG_MANAGER_HPP

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <expected>
#include <vector>

namespace cgui {

/**
 * @brief Manages encrypted configuration files.
 */
class ConfigManager {
public:
    ConfigManager() = default;
    ~ConfigManager();

    // Prevent copying
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    /**
     * @brief Load and decrypt an INI file.
     * @param path Path to the encrypted INI file.
     * @param password Password for decryption.
     * @return Success or error message.
     */
    [[nodiscard]] std::expected<void, std::string> load_encrypted_ini(const std::filesystem::path& path, const std::string& password);

    /**
     * @brief Get the loaded configuration.
     * @return JSON object containing the configuration.
     */
    [[nodiscard]] const nlohmann::json& get_config() const;

private:
    nlohmann::json m_config;

    /**
     * @brief Securely clear sensitive data.
     * @param data String to clear.
     */
    void secure_clear(std::string& data);

    /**
     * @brief Securely clear sensitive data.
     * @param data Vector to clear.
     */
    void secure_clear(std::vector<unsigned char>& data);
};

} // namespace cgui

#endif // C_GUI_CONFIG_MANAGER_HPP
