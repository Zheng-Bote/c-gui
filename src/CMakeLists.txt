# SPDX-FileComment: CMakeLists.txt for the main application
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file src/CMakeLists.txt
# @brief Source directory build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

# --- Find Dependencies ---
find_package(wxWidgets REQUIRED COMPONENTS core base net xml)

find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(libsodium REQUIRED)
find_package(valijson REQUIRED)
find_package(inih REQUIRED)
find_package(spdlog REQUIRED)

# --- Core Library ---
set(CORE_SOURCES
    AppController.cpp
    MainWindow.cpp
    ConfigManager.cpp
    LogManager.cpp
    AuthManager.cpp
    PluginLoader.cpp
    TopicRegistry.cpp
    SchemaManager.cpp
    Validator.cpp
    Uploader.cpp
)

add_library(cgui_core STATIC ${CORE_SOURCES})

target_include_directories(cgui_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cgui_core PUBLIC
    wxWidgets::wxWidgets
    nlohmann_json::nlohmann_json
    CURL::libcurl
    libsodium::libsodium
    ValiJSON::valijson
    inih::inih
    spdlog::spdlog
    gh_update_checker::gh_update_checker
)

# --- Executable ---
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE cgui_core)

# --- Encryption Tool ---
add_executable(encrypt_tool encrypt_tool.cpp)
target_link_libraries(encrypt_tool PRIVATE libsodium::libsodium)
target_include_directories(encrypt_tool PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# --- Platform Specifics ---
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "net.hase-zheng.c-gui"
        MACOSX_BUNDLE_BUNDLE_NAME "Cority GUI"
    )
elseif(WIN32)
    target_link_libraries(cgui_core PUBLIC Wininet) # Often needed for wxWidgets/curl
endif()
