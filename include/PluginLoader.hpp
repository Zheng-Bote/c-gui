/**
 * SPDX-FileComment: PluginLoader for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file PluginLoader.hpp
 * @brief Dynamic library loader for plugins.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_PLUGIN_LOADER_HPP
#define C_GUI_PLUGIN_LOADER_HPP

#include <string>
#include <filesystem>
#include <memory>
#include <expected>
#include "PluginAPI.hpp"

namespace cgui {

/**
 * @brief Loads and manages a single plugin from a shared library.
 */
class PluginLoader {
public:
    PluginLoader() = default;
    ~PluginLoader();

    // Prevent copying
    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;

    /**
     * @brief Load a plugin from the specified path.
     * @param path Path to the shared library.
     * @return Success or error message.
     */
    [[nodiscard]] std::expected<void, std::string> load(const std::filesystem::path& path);

    /**
     * @brief Unload the plugin.
     */
    void unload();

    /**
     * @brief Get the loaded plugin instance.
     * @return Pointer to the plugin instance.
     */
    [[nodiscard]] IPlugin* get_plugin() const;

private:
    void* m_handle = nullptr;
    IPlugin* m_plugin = nullptr;

    using CreatePluginFunc = IPlugin* (*)();
    using DestroyPluginFunc = void (*)(IPlugin*);

    DestroyPluginFunc m_destroy_func = nullptr;
};

} // namespace cgui

#endif // C_GUI_PLUGIN_LOADER_HPP
