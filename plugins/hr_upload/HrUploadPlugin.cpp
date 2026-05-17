/**
 * SPDX-FileComment: HR Upload Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrUploadPlugin.cpp
 * @brief Implementation of HR-specific upload plugin with Cority Auth flow.
 * @version 1.2.0
 * @date 2026-05-17
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "HrUploadPlugin.hpp"
#include <curl/curl.h>
#include <format>
#include <iostream>

namespace cgui {

bool HrUploadPlugin::initialize(const nlohmann::json& config) {
    m_config = config;
    
    // 1. Try to get from top level
    if (config.contains("base_url") && !config["base_url"].is_null()) m_base_url = config["base_url"].get<std::string>();
    if (config.contains("login") && !config["login"].is_null()) m_login = config["login"].get<std::string>();
    if (config.contains("password") && !config["password"].is_null()) m_password = config["password"].get<std::string>();

    // 2. Try to get from [auth] section of INI (if provided)
    if (config.contains("auth") && config["auth"].is_object()) {
        const auto& auth = config["auth"];
        if (m_base_url.empty() && auth.contains("saas_base_url") && !auth["saas_base_url"].is_null()) 
            m_base_url = auth["saas_base_url"].get<std::string>();
        if (m_login.empty() && auth.contains("saas_login") && !auth["saas_login"].is_null()) 
            m_login = auth["saas_login"].get<std::string>();
        if (m_password.empty() && auth.contains("saas_password") && !auth["saas_password"].is_null()) 
            m_password = auth["saas_password"].get<std::string>();
    }

    // 3. Fallback: derive base_url from upload_endpoint if still missing
    if (m_base_url.empty() && config.contains("upload_endpoint") && !config["upload_endpoint"].is_null()) {
        std::string ep = config["upload_endpoint"].get<std::string>();
        auto pos = ep.find("/api/");
        if (pos != std::string::npos) {
            m_base_url = ep.substr(0, pos);
        }
    }

    if (!m_base_url.empty() && m_base_url.back() == '/') {
        m_base_url.pop_back();
    }

    if (m_base_url.empty() || m_login.empty() || m_password.empty()) {
        std::cerr << "HrUploadPlugin: Missing required config (base_url, login, or password)" << std::endl;
        return false;
    }

    return true;
}

size_t HrUploadPlugin::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::expected<void, std::string> HrUploadPlugin::ensure_authenticated() {
    auto now = std::chrono::system_clock::now();
    
    // Margin of 60 seconds like in uploader.py
    if (!m_access_token.empty() && m_access_expiry > (now + std::chrono::seconds(60))) {
        return {};
    }

    // Need to re-auth
    auto refresh_res = perform_refresh();
    if (!refresh_res) return refresh_res;

    return fetch_access_token();
}

std::expected<void, std::string> HrUploadPlugin::perform_refresh() {
    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("Failed to initialize curl");

    std::string url = m_base_url + "/api/refreshtoken";
    nlohmann::json payload = {
        {"user", {
            {"LoginName", m_login},
            {"Loginpassword", m_password}
        }}
    };
    std::string payload_str = payload.dump();
    std::string response_string;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return std::unexpected(curl_easy_strerror(res));
    if (response_code != 200) return std::unexpected(std::format("Refresh failed with status {}: {}", response_code, response_string));

    try {
        auto data = nlohmann::json::parse(response_string);
        if (data.contains("Token") && !data["Token"].is_null()) {
            m_refresh_token = data["Token"].get<std::string>();
            return {};
        }
    } catch (...) {}

    return std::unexpected("Refresh token not found in response: " + response_string);
}

std::expected<void, std::string> HrUploadPlugin::fetch_access_token() {
    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("Failed to initialize curl");

    std::string url = m_base_url + "/api/token/";
    std::string response_string;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, std::format("Authorization: Bearer {}", m_refresh_token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return std::unexpected(curl_easy_strerror(res));
    if (response_code != 200) return std::unexpected(std::format("Access token request failed with status {}: {}", response_code, response_string));

    try {
        auto data = nlohmann::json::parse(response_string);
        if (data.contains("AccessToken") && !data["AccessToken"].is_null()) {
            m_access_token = data["AccessToken"].get<std::string>();
            
            // Note: Simple expiry handling (default 1 hour if not parsed)
            // Parsing ISO date in C++23 is possible but verbose, 
            // for now we use a safe default or 0 if we want to force refresh next time.
            m_access_expiry = std::chrono::system_clock::now() + std::chrono::minutes(55); 
            
            return {};
        }
    } catch (...) {}

    return std::unexpected("Access token not found in response: " + response_string);
}

std::expected<void, std::string> HrUploadPlugin::upload(const nlohmann::json& data) {
    auto auth_res = ensure_authenticated();
    if (!auth_res) return auth_res;

    nlohmann::json upload_payload;
    
    // Formatting: Ensure we have "options" and "records"
    if (data.contains("options") && data.contains("records")) {
        upload_payload = data;
    } else {
        // Default options like in hr_upload_example.json
        nlohmann::json options = {
            {"updateExistingRecords", "true"},
            {"insertBaseTables", "true"},
            {"forceLookupTableUpdate", "true"},
            {"disableSegUpdate", "false"},
            {"autoCreatePortalUser", "true"},
            {"mergeRecordsWithMatchingSsn", "false"},
            {"dateFormat", "dd.mm.yyyy"}
        };

        // Allow override from config
        if (m_config.contains("options") && m_config["options"].is_object()) {
            for (auto& [key, value] : m_config["options"].items()) {
                options[key] = value;
            }
        }

        upload_payload["options"] = options;
        upload_payload["records"] = data; // data is the list of records
    }

    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("Failed to initialize curl");

    std::string url = m_base_url + "/api/employeeimport";
    std::string payload_str = upload_payload.dump();
    std::string response_string;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, std::format("Authorization: Bearer {}", m_access_token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return std::unexpected(curl_easy_strerror(res));
    
    if (response_code == 401) {
        // Token might have expired unexpectedly, force re-auth once
        m_access_token.clear();
        return upload(data); 
    }

    if (response_code < 200 || response_code >= 300) {
        return std::unexpected(std::format("Upload failed with status {}: {}", response_code, response_string));
    }

    return {};
}

void HrUploadPlugin::shutdown() {}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::HrUploadPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
