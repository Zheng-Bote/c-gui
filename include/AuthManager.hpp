/**
 * SPDX-FileComment: AuthManager for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file AuthManager.hpp
 * @brief Handles authentication with SaaS provider to obtain tokens.
 * @version 1.0.0
 * @date 2026-05-14
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_AUTH_MANAGER_HPP
#define C_GUI_AUTH_MANAGER_HPP

#include <string>
#include <expected>
#include <nlohmann/json.hpp>

namespace cgui {

/**
 * @struct AuthTokens
 * @brief Holds access and refresh tokens.
 */
struct AuthTokens {
    std::string access_token;
    std::string refresh_token;
    std::string access_expiry;
};

/**
 * @class AuthManager
 * @brief Manages authentication flow (refresh token -> access token).
 */
class AuthManager {
public:
    /**
     * @brief Authenticate and obtain tokens.
     * @param base_url SaaS base URL.
     * @param login Login name.
     * @param password Password.
     * @return AuthTokens or error message.
     */
    static std::expected<AuthTokens, std::string> authenticate(const std::string& base_url, 
                                                              const std::string& login, 
                                                              const std::string& password);

    /**
     * @brief Obtain a new access token using a refresh token.
     * @param base_url SaaS base URL.
     * @param refresh_token Valid refresh token.
     * @return AuthTokens (updated) or error message.
     */
    static std::expected<AuthTokens, std::string> refresh_access_token(const std::string& base_url, 
                                                                     const std::string& refresh_token);
};

} // namespace cgui

#endif // C_GUI_AUTH_MANAGER_HPP
