# SPDX-FileComment: CMakeLists.txt for Kafka Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file plugins/kafka_input/CMakeLists.txt
# @brief Kafka input plugin build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

add_library(hr_kafka_input SHARED KafkaInputPlugin.cpp KafkaInputPlugin.hpp)

target_include_directories(hr_kafka_input PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find dependencies
find_package(nlohmann_json REQUIRED)
find_package(RdKafka REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(hr_kafka_input PRIVATE 
    cgui_core
    nlohmann_json::nlohmann_json
    RdKafka::rdkafka++
    spdlog::spdlog
)

set_target_properties(hr_kafka_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
