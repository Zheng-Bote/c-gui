/**
 * SPDX-FileComment: HR Upload Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrUploadPlugin.cpp
 * @brief Implementation of HR-specific upload plugin.
 * @version 1.0.0
 * @date 2026-05-15
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "HrUploadPlugin.hpp"
#include <curl/curl.h>

namespace cgui {

bool HrUploadPlugin::initialize(const nlohmann::json& config) {
    m_config = config;
    if (config.contains("upload_endpoint")) {
        m_endpoint = config["upload_endpoint"].get<std::string>();
    }

    if (config.contains("auth") && config["auth"].contains("bearer_token")) {
        m_bearer_token = config["auth"]["bearer_token"].get<std::string>();
    }

    return !m_endpoint.empty();
}

size_t HrUploadPlugin::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::expected<void, std::string> HrUploadPlugin::upload(const nlohmann::json& data) {
    // HR-specific formatting
    nlohmann::json upload_payload = nlohmann::json::object();
    upload_payload["options"] = {
        {"updateExistingRecords", "true"},
        {"insertBaseTables", "true"},
        {"forceLookupTableUpdate", "true"},
        {"disableSegUpdate", "false"},
        {"autoCreatePortalUser", "true"},
        {"mergeRecordsWithMatchingSsn", "false"},
        {"dateFormat", "dd.mm.yyyy"}
    };
    upload_payload["records"] = data;

    // Perform upload using libcurl
    CURL* curl = curl_easy_init();
    if (!curl) {
        return std::unexpected("Failed to initialize curl");
    }

    std::string response_string;
    std::string payload_str = upload_payload.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!m_bearer_token.empty()) {
        std::string auth_header = "Authorization: Bearer " + m_bearer_token;
        headers = curl_slist_append(headers, auth_header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, m_endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return std::unexpected(curl_easy_strerror(res));
    }

    if (response_code < 200 || response_code >= 300) {
        return std::unexpected("HTTP Error: " + std::to_string(response_code) + " - " + response_string);
    }

    return {};
}

void HrUploadPlugin::shutdown() {
    // No specific cleanup needed
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::HrUploadPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
