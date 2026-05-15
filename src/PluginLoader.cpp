/**
 * SPDX-FileComment: PluginLoader implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file PluginLoader.cpp
 * @brief Implementation of dynamic library loader for plugins.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "PluginLoader.hpp"
#include "LogManager.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace cgui {

PluginLoader::~PluginLoader() {
    unload();
}

std::expected<void, std::string> PluginLoader::load(const std::filesystem::path& path) {
    auto logger = LogManager::get_instance().get_core_logger();
    logger->info("Loading plugin from: {}", path.string());
    unload();

#ifdef _WIN32
    m_handle = LoadLibraryW(path.wstring().c_str());
    if (!m_handle) {
        logger->error("Failed to load library: {} (error code: {})", path.string(), GetLastError());
        return std::unexpected("Failed to load library: " + std::to_string(GetLastError()));
    }
#else
    m_handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!m_handle) {
        const char* err = dlerror();
        std::string err_msg = err ? err : "Unknown error";
        logger->error("Failed to load library: {} (error: {})", path.string(), err_msg);
        return std::unexpected("Failed to load library: " + err_msg);
    }
#endif

#ifdef _WIN32
    auto create_func = reinterpret_cast<CreatePluginFunc>(GetProcAddress(static_cast<HMODULE>(m_handle), "create_plugin"));
    m_destroy_func = reinterpret_cast<DestroyPluginFunc>(GetProcAddress(static_cast<HMODULE>(m_handle), "destroy_plugin"));
#else
    auto create_func = reinterpret_cast<CreatePluginFunc>(dlsym(m_handle, "create_plugin"));
    m_destroy_func = reinterpret_cast<DestroyPluginFunc>(dlsym(m_handle, "destroy_plugin"));
    
    if (!create_func || !m_destroy_func) {
        const char* err = dlerror();
        std::string err_msg = err ? err : "Symbol not found";
        logger->error("Failed to find required symbols in plugin: {} (error: {})", path.string(), err_msg);
        unload();
        return std::unexpected("Failed to find required symbols: " + err_msg);
    }
#endif

    m_plugin = create_func();
    if (!m_plugin) {
        logger->error("Plugin factory failed to create plugin instance: {}", path.string());
        unload();
        return std::unexpected("Plugin factory failed to create plugin instance");
    }

    logger->info("Plugin loaded successfully: {}", path.string());
    return {};
}

void PluginLoader::unload() {
    if (m_plugin && m_destroy_func) {
        LogManager::get_instance().get_core_logger()->info("Unloading plugin instance");
        m_destroy_func(m_plugin);
        m_plugin = nullptr;
    }

    if (m_handle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(m_handle));
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
    }
    m_destroy_func = nullptr;
}

IPlugin* PluginLoader::get_plugin() const {
    return m_plugin;
}

} // namespace cgui
