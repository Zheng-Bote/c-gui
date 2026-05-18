/**
 * SPDX-FileComment: Uploader implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file Uploader.cpp
 * @brief Implementation of libcurl-based HTTP uploader.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "Uploader.hpp"
#include "LogManager.hpp"
#include <curl/curl.h>
#include <mutex>

namespace cgui {

static std::once_flag curl_init_flag;

Uploader::Uploader() {
    std::call_once(curl_init_flag, []() {
        curl_global_init(CURL_GLOBAL_ALL);
    });
}

Uploader::~Uploader() {
    // curl_global_cleanup() is usually called at app exit
}

size_t Uploader::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::expected<void, std::string> Uploader::upload_sync(const std::string& url, const nlohmann::json& payload, const std::string& bearer_token) {
    auto logger = LogManager::get_instance().get_core_logger();
    logger->info("Starting synchronous upload to: {}", url);

    CURL* curl = curl_easy_init();
    if (!curl) {
        logger->error("Failed to initialize curl for upload to: {}", url);
        return std::unexpected("Failed to initialize curl");
    }

    std::string response_string;
    std::string data = payload.dump();
    logger->debug("Payload size: {} bytes", data.length());
    logger->debug("Payload preview: {}", data.substr(0, 1000));

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!bearer_token.empty()) {
        std::string auth_header = "Authorization: Bearer " + bearer_token;
        headers = curl_slist_append(headers, auth_header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_timeout);

    if (!m_verify_ssl) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    if (!m_ssl_ca_path.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, m_ssl_ca_path.c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::string error_msg = curl_easy_strerror(res);
        logger->error("Upload to {} failed: {}", url, error_msg);
        return std::unexpected(error_msg);
    }

    if (response_code < 200 || response_code >= 300) {
        logger->error("Upload to {} failed with HTTP error code: {}", url, response_code);
        logger->error("Server response body: {}", response_string);
        return std::unexpected("HTTP Error: " + std::to_string(response_code) + " - " + response_string);
    }

    logger->info("Upload to {} successful (HTTP {})", url, response_code);
    return {};
}

void Uploader::upload_async(const std::string& url, const nlohmann::json& payload, 
                            std::function<void(std::expected<void, std::string>)> callback,
                            const std::string& bearer_token) {
    LogManager::get_instance().get_core_logger()->info("Starting asynchronous upload to: {}", url);
    std::thread([this, url, payload, callback, bearer_token]() {
        callback(upload_sync(url, payload, bearer_token));
    }).detach();
}

void Uploader::configure_ssl(bool verify_ssl, const std::string& ca_path) {
    m_verify_ssl = verify_ssl;
    m_ssl_ca_path = ca_path;
}

void Uploader::set_timeout(long timeout_seconds) {
    m_timeout = timeout_seconds;
}

} // namespace cgui
