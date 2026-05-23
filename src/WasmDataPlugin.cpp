/**
 * SPDX-FileComment: WASM Data Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file WasmDataPlugin.cpp
 * @brief Implementation of IDataPlugin for WASM modules.
 * @version 1.0.0
 * @date 2025-05-23
 */

#include "WasmDataPlugin.hpp"
#include "LogManager.hpp"
#include <fstream>
#include <vector>
#include <curl/curl.h>

namespace cgui {

namespace {
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
}

WasmDataPlugin::WasmDataPlugin(const std::filesystem::path& wasm_path) 
    : m_store(m_engine) {
    auto logger = LogManager::get_instance().get_core_logger();
    
    try {
        std::ifstream file(wasm_path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Could not open WASM file: " + wasm_path.string());
        }
        std::vector<uint8_t> wasm_bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        auto module = wasmtime::Module::compile(m_engine, wasm_bytes).unwrap();
        
        // Setup Linker and WASI
        wasmtime::Linker linker(m_engine);
        linker.define_wasi().unwrap();

        // Add Host Networking Helper
        linker.define(m_store.context(), "env", "host_http_get", wasmtime::Func::wrap(m_store, [this](wasmtime::Caller caller, int32_t url_ptr, int32_t url_len) -> int32_t {
            auto mem_extern = caller.get_export("memory");
            if (!mem_extern || !std::holds_alternative<wasmtime::Memory>(*mem_extern)) return 0;
            auto mem = std::get<wasmtime::Memory>(*mem_extern);
            
            auto data_span = mem.data(m_store);
            if (url_ptr < 0 || static_cast<size_t>(url_ptr + url_len) > data_span.size()) return 0;
            
            std::string url(reinterpret_cast<const char*>(data_span.data() + url_ptr), url_len);
            
            LogManager::get_instance().get_core_logger()->debug("WASM requested HTTP GET: {}", url);

            std::string response;
            CURL* curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

                // Use system trust store (especially important on Windows)
                #ifdef _WIN32
                curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
                #endif

                if (!this->m_proxy.empty()) {                    curl_easy_setopt(curl, CURLOPT_PROXY, this->m_proxy.c_str());
                    LogManager::get_instance().get_core_logger()->debug("Using proxy for WASM HTTP GET: {}", this->m_proxy);
                }
                
                CURLcode res = curl_easy_perform(curl);
                
                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                curl_easy_cleanup(curl);
                
                if (res != CURLE_OK) {
                    std::string err_msg = std::format("WASM HTTP GET failed for {}: {} (CURLcode: {})", url, curl_easy_strerror(res), (int)res);
                    LogManager::get_instance().get_core_logger()->error(err_msg);
                    
                    // If SSL fails, maybe it's missing CA certs on Windows
                    if (res == CURLE_PEER_FAILED_VERIFICATION || res == CURLE_SSL_CACERT) {
                        LogManager::get_instance().get_core_logger()->warn("SSL Verification failed. Check system CA certificates or proxy configuration.");
                    }
                    return 0;
                }
                
                if (http_code >= 400) {
                    LogManager::get_instance().get_core_logger()->error("WASM HTTP GET returned error code {} for {}", http_code, url);
                    return 0;
                }
                
                LogManager::get_instance().get_core_logger()->debug("WASM HTTP GET successful ({} bytes) from {}", response.length(), url);
            }

            // Copy back to WASM
            auto alloc_extern = caller.get_export("allocate");
            if (!alloc_extern || !std::holds_alternative<wasmtime::Func>(*alloc_extern)) return 0;
            auto alloc = std::get<wasmtime::Func>(*alloc_extern);
            
            auto res_wrap = alloc.call(m_store, {(int32_t)response.length()});
            if (!res_wrap) return 0;
            
            auto res = res_wrap.unwrap();
            int32_t guest_ptr = res[0].i32();
            
            // Re-fetch memory as it might have grown during alloc call
            auto data_span_new = mem.data(m_store);
            if (guest_ptr < 0 || static_cast<size_t>(guest_ptr + response.length()) > data_span_new.size()) return 0;
            
            std::memcpy(data_span_new.data() + guest_ptr, response.data(), response.length());
            
            this->m_last_http_len = response.length();
            return guest_ptr;
        })).unwrap();

        linker.define(m_store.context(), "env", "host_http_get_len", wasmtime::Func::wrap(m_store, [this](wasmtime::Caller /*caller*/) -> int32_t {
            return (int32_t)this->m_last_http_len;
        })).unwrap();

        wasmtime::WasiConfig wasi;
        wasi.inherit_stdout();
        wasi.inherit_stderr();
        m_store.context().set_wasi(std::move(wasi)).unwrap();

        auto instance = linker.instantiate(m_store, module).unwrap();
        m_instance = std::make_unique<wasmtime::Instance>(std::move(instance));
        
        auto get_export = [&](const std::string& name) -> std::optional<wasmtime::Extern> {
            return m_instance->get(m_store, name);
        };

        auto mem_extern = get_export("memory");
        if (mem_extern && std::holds_alternative<wasmtime::Memory>(*mem_extern)) {
            m_memory = std::get<wasmtime::Memory>(*mem_extern);
        }
        
        if (!m_memory) {
            throw std::runtime_error("WASM module has no exported memory");
        }

        auto alloc_extern = get_export("allocate");
        if (alloc_extern && std::holds_alternative<wasmtime::Func>(*alloc_extern)) {
            m_allocate = std::get<wasmtime::Func>(*alloc_extern);
        }

        auto dealloc_extern = get_export("deallocate");
        if (dealloc_extern && std::holds_alternative<wasmtime::Func>(*dealloc_extern)) {
            m_deallocate = std::get<wasmtime::Func>(*dealloc_extern);
        }
        
        logger->info("WASM module loaded successfully from {}", wasm_path.string());
    } catch (const std::exception& e) {
        logger->error("Failed to load WASM plugin: {}", e.what());
        throw;
    }
}

WasmDataPlugin::~WasmDataPlugin() {
    shutdown();
}

int32_t WasmDataPlugin::copy_to_wasm(const std::string& str) {
    if (!m_allocate || !m_memory) return -1;
    
    auto result = m_allocate->call(m_store, {(int32_t)str.length()}).unwrap();
    int32_t ptr = result[0].i32();
    
    auto data_span = m_memory->data(m_store);
    std::copy(str.begin(), str.end(), data_span.data() + ptr);
    
    return ptr;
}

bool WasmDataPlugin::initialize(const nlohmann::json& config) {
    if (config.contains("proxy") && config["proxy"].is_string()) {
        m_proxy = config["proxy"].get<std::string>();
    }

    auto init_extern = m_instance->get(m_store, "initialize");
    if (!init_extern || !std::holds_alternative<wasmtime::Func>(*init_extern)) return false;
    auto init_func = std::get<wasmtime::Func>(*init_extern);
    
    std::string config_str = config.dump();
    int32_t ptr = copy_to_wasm(config_str);
    if (ptr < 0) return false;
    
    auto result = init_func.call(m_store, {ptr, (int32_t)config_str.length()}).unwrap();
    
    if (m_deallocate) {
        m_deallocate->call(m_store, {ptr, (int32_t)config_str.length()}).unwrap();
    }
    
    return result[0].i32() != 0;
}

std::string WasmDataPlugin::call_wasm_string_func(const std::string& func_name) const {
    try {
        auto func_extern = m_instance->get(const_cast<wasmtime::Store&>(m_store), func_name);
        if (!func_extern || !std::holds_alternative<wasmtime::Func>(*func_extern) || !m_memory) return "";
        auto func = std::get<wasmtime::Func>(*func_extern);
        
        auto result = func.call(const_cast<wasmtime::Store&>(m_store), {}).unwrap();
        int32_t ptr = result[0].i32();
        
        auto len_extern = m_instance->get(const_cast<wasmtime::Store&>(m_store), "get_last_result_len");
        if (!len_extern || !std::holds_alternative<wasmtime::Func>(*len_extern)) return "";
        auto len_func = std::get<wasmtime::Func>(*len_extern);
        
        auto len_result = len_func.call(const_cast<wasmtime::Store&>(m_store), {}).unwrap();
        int32_t len = len_result[0].i32();
        
        auto data_span = m_memory->data(const_cast<wasmtime::Store&>(m_store));
        if (ptr < 0 || static_cast<size_t>(ptr + len) > data_span.size()) {
            return "";
        }
        return std::string(reinterpret_cast<const char*>(data_span.data() + ptr), len);
    } catch (const std::exception& e) {
        LogManager::get_instance().get_core_logger()->error("WASM error in {}: {}", func_name, e.what());
        return "";
    }
}

std::string WasmDataPlugin::get_topic() const {
    return call_wasm_string_func("get_topic");
}

std::string WasmDataPlugin::get_version() const {
    return call_wasm_string_func("get_version");
}

std::string WasmDataPlugin::get_interface_type() const {
    return call_wasm_string_func("get_interface_type");
}

void WasmDataPlugin::shutdown() {
    try {
        auto func_extern = m_instance->get(m_store, "shutdown");
        if (func_extern && std::holds_alternative<wasmtime::Func>(*func_extern)) {
            std::get<wasmtime::Func>(*func_extern).call(m_store, {}).unwrap();
        }
    } catch (...) {
        // Silently fail during shutdown
    }
}

std::vector<nlohmann::json> WasmDataPlugin::fetch_batch(size_t max_records) {
    try {
        auto func_extern = m_instance->get(m_store, "fetch_batch");
        if (!func_extern || !std::holds_alternative<wasmtime::Func>(*func_extern) || !m_memory) return {};
        auto func = std::get<wasmtime::Func>(*func_extern);
        
        auto result = func.call(m_store, {(int32_t)max_records}).unwrap();
        int32_t ptr = result[0].i32();
        
        auto len_extern = m_instance->get(m_store, "get_last_result_len");
        if (!len_extern || !std::holds_alternative<wasmtime::Func>(*len_extern)) return {};
        auto len_func = std::get<wasmtime::Func>(*len_extern);
        
        auto len_result = len_func.call(m_store, {}).unwrap();
        int32_t len = len_result[0].i32();
        
        auto data_span = m_memory->data(m_store);
        if (ptr < 0 || static_cast<size_t>(ptr + len) > data_span.size()) {
            return {};
        }
        std::string json_str(reinterpret_cast<const char*>(data_span.data() + ptr), len);
        
        auto j = nlohmann::json::parse(json_str);
        if (j.is_array()) {
            return j.get<std::vector<nlohmann::json>>();
        }
    } catch (const std::exception& e) {
        LogManager::get_instance().get_core_logger()->error("WASM error in fetch_batch: {}", e.what());
    } catch (...) {
        LogManager::get_instance().get_core_logger()->error("Unknown error in fetch_batch");
    }
    
    return {};
}

} // namespace cgui
