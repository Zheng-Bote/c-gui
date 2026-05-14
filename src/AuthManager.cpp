/**
 * SPDX-FileComment: AuthManager implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file AuthManager.cpp
 * @brief Implementation of authentication flow.
 * @version 1.0.0
 * @date 2026-05-14
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "AuthManager.hpp"
#include "LogManager.hpp"
#include <curl/curl.h>
#include <format>

namespace cgui {

namespace {
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::expected<nlohmann::json, std::string> http_post(const std::string& url, const nlohmann::json& payload, const std::string& auth_header = "") {
    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("Failed to initialize curl");

    std::string response_string;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!auth_header.empty()) {
        headers = curl_slist_append(headers, auth_header.c_str());
    }

    std::string body = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return std::unexpected(std::format("HTTP request failed: {}", curl_easy_strerror(res)));
    }

    if (response_code >= 400) {
        return std::unexpected(std::format("HTTP error {}: {}", response_code, response_string));
    }

    try {
        return nlohmann::json::parse(response_string);
    } catch (const std::exception& e) {
        return std::unexpected(std::format("Failed to parse JSON response: {}", e.what()));
    }
}

std::expected<nlohmann::json, std::string> http_get(const std::string& url, const std::string& auth_header) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("Failed to initialize curl");

    std::string response_string;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return std::unexpected(std::format("HTTP request failed: {}", curl_easy_strerror(res)));
    }

    if (response_code >= 400) {
        return std::unexpected(std::format("HTTP error {}: {}", response_code, response_string));
    }

    try {
        return nlohmann::json::parse(response_string);
    } catch (const std::exception& e) {
        return std::unexpected(std::format("Failed to parse JSON response: {}", e.what()));
    }
}
} // namespace anonymous

std::expected<AuthTokens, std::string> AuthManager::authenticate(const std::string& base_url, 
                                                              const std::string& login, 
                                                              const std::string& password) {
    auto logger = LogManager::get_instance().get_core_logger();
    logger->info("Authenticating against {} for user {}", base_url, login);

    // 1) POST /api/refreshtoken
    std::string refresh_url = base_url;
    if (refresh_url.back() == '/') refresh_url.pop_back();
    refresh_url += "/api/refreshtoken";

    nlohmann::json refresh_payload;
    refresh_payload["user"]["LoginName"] = login;
    refresh_payload["user"]["Loginpassword"] = password;

    auto refresh_res = http_post(refresh_url, refresh_payload);
    if (!refresh_res) {
        logger->error("Refresh token request failed: {}", refresh_res.error());
        return std::unexpected(refresh_res.error());
    }

    logger->debug("Refresh token response received successfully.");

    std::string refresh_token;
    if (refresh_res->contains("Token")) refresh_token = (*refresh_res)["Token"].get<std::string>();
    else if (refresh_res->contains("token")) refresh_token = (*refresh_res)["token"].get<std::string>();
    
    if (refresh_token.empty()) {
        logger->error("Refresh token not found in JSON response: {}", refresh_res->dump());
        return std::unexpected("Refresh token not found in response");
    }

    logger->info("Refresh token obtained (len={}). Requesting access token...", refresh_token.length());
    return refresh_access_token(base_url, refresh_token);
}

std::expected<AuthTokens, std::string> AuthManager::refresh_access_token(const std::string& base_url, 
                                                                     const std::string& refresh_token) {
    auto logger = LogManager::get_instance().get_core_logger();
    logger->info("Requesting access token from /api/token/...");

    // 2) GET /api/token/
    std::string access_url = base_url;
    if (access_url.back() == '/') access_url.pop_back();
    access_url += "/api/token/";

    std::string auth_header = std::format("Authorization: Bearer {}", refresh_token);
    auto access_res = http_get(access_url, auth_header);
    if (!access_res) {
        logger->error("Access token request failed: {}", access_res.error());
        return std::unexpected(access_res.error());
    }

    logger->debug("Access token response received successfully.");

    AuthTokens tokens;
    tokens.refresh_token = refresh_token;
    
    if (access_res->contains("AccessToken")) tokens.access_token = (*access_res)["AccessToken"].get<std::string>();
    else if (access_res->contains("access_token")) tokens.access_token = (*access_res)["access_token"].get<std::string>();
    else if (access_res->contains("token")) tokens.access_token = (*access_res)["token"].get<std::string>();

    if (access_res->contains("AccessTokenExpiryDateTime")) {
        tokens.access_expiry = (*access_res)["AccessTokenExpiryDateTime"].get<std::string>();
        logger->debug("Access token expiry: {}", tokens.access_expiry);
    }

    if (tokens.access_token.empty()) {
        logger->error("Access token not found in JSON response: {}", access_res->dump());
        return std::unexpected("Access token not found in response");
    }

    logger->info("Access token obtained successfully (len={}).", tokens.access_token.length());
    return tokens;
}

} // namespace cgui
