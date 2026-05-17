/**
 * SPDX-FileComment: Unit tests for PluginLoader
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file tests/test_plugin_loader.cpp
 * @brief Unit tests for the PluginLoader class
 * @version 1.0.0
 * @date 2026-05-14
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "PluginLoader.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>

TEST_CASE("PluginLoader loads and unloads a plugin", "[PluginLoader]") {
  cgui::PluginLoader loader;
  std::string plugin_path = DUMMY_PLUGIN_PATH;

  REQUIRE(std::filesystem::exists(plugin_path));

  auto res = loader.load(plugin_path);
  REQUIRE(res.has_value());

  auto *plugin = loader.get_plugin();
  REQUIRE(plugin != nullptr);

  SECTION("Plugin can be initialized") {
    nlohmann::json config = {{"topic", "test_topic"}};
    REQUIRE(plugin->initialize(config) == true);
  }

  loader.unload();
}

TEST_CASE("PluginLoader handles non-existent plugin", "[PluginLoader]") {
  cgui::PluginLoader loader;
  auto res = loader.load("non_existent_path.so");
  REQUIRE(!res.has_value());
}
