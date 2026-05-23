/**
 * SPDX-FileComment: WASM Data Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file WasmDataPlugin.hpp
 * @brief IDataPlugin implementation that hosts a WebAssembly module.
 * @version 1.0.0
 * @date 2025-05-23
 */

#ifndef C_GUI_WASM_DATA_PLUGIN_HPP
#define C_GUI_WASM_DATA_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <wasmtime.hh>
#include <filesystem>
#include <memory>

namespace cgui {

/**
 * @brief implementation of IDataPlugin that executes a WASM module.
 * 
 * Expected WASM exports:
 * - allocate(size: i32) -> i32
 * - deallocate(ptr: i32, size: i32)
 * - initialize(config_ptr: i32, config_len: i32) -> i32 (bool)
 * - fetch_batch(max_records: i32) -> i32 (returns pointer to JSON string)
 * - get_last_result_len() -> i32
 * - get_topic() -> i32 (returns pointer)
 * - get_version() -> i32 (returns pointer)
 * - get_interface_type() -> i32 (returns pointer)
 * - shutdown()
 */
class WasmDataPlugin : public IDataPlugin {
public:
    explicit WasmDataPlugin(const std::filesystem::path& wasm_path);
    ~WasmDataPlugin() override;

    bool initialize(const nlohmann::json& config) override;
    std::string get_topic() const override;
    std::string get_version() const override;
    void shutdown() override;
    
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    std::string get_interface_type() const override;

private:
    std::string call_wasm_string_func(const std::string& func_name) const;
    int32_t copy_to_wasm(const std::string& str);

    wasmtime::Engine m_engine;
    wasmtime::Store m_store;
    std::unique_ptr<wasmtime::Instance> m_instance;
    std::optional<wasmtime::Memory> m_memory;

    // Cached functions
    std::optional<wasmtime::Func> m_allocate;
    std::optional<wasmtime::Func> m_deallocate;

    size_t m_last_http_len = 0;
    std::string m_proxy;
};

} // namespace cgui

#endif // C_GUI_WASM_DATA_PLUGIN_HPP
