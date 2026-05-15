/**
 * SPDX-FileComment: LogManager implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file LogManager.cpp
 * @brief Implementation of topic-based logging management.
 * @version 1.0.0
 * @date 2026-05-14
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "LogManager.hpp"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/pattern_formatter.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>

namespace cgui {

namespace {
    LogManager::LogCallback g_callback = nullptr;
}

LogManager& LogManager::get_instance() {
    static LogManager instance;
    return instance;
}

void LogManager::initialize(const std::string& log_path, const std::string& log_level) {
    m_log_path = log_path;
    m_level = parse_level(log_level);
    
    if (!std::filesystem::exists(m_log_path)) {
        std::filesystem::create_directories(m_log_path);
    }

    // Initialize core logger
    get_core_logger();
}

void LogManager::set_callback(LogCallback callback) {
    g_callback = callback;
}

std::shared_ptr<spdlog::logger> LogManager::get_logger(const std::string& topic) {
    std::string date_str = get_current_date_str();
    std::string logger_name = date_str + "_" + topic;

    auto it = m_loggers.find(logger_name);
    if (it != m_loggers.end()) {
        return it->second;
    }

    // Create new logger for this topic and date
    std::string filename = (std::filesystem::path(m_log_path) / (logger_name + ".log")).string();
    
    // 10MB rotation, max 5 files
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, 1024 * 1024 * 10, 5);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};

    if (g_callback) {
        auto gui_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg) {
            if (!g_callback) return;
            
            spdlog::memory_buf_t formatted;
            spdlog::pattern_formatter formatter("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
            
            formatter.format(msg, formatted);
            
            g_callback(fmt::to_string(formatted));
        });
        sinks.push_back(gui_sink);
    }

    auto logger = std::make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());
    
    logger->set_level(m_level);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
    
    // Ensure logs are flushed immediately
    logger->flush_on(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1));
    
    m_loggers[logger_name] = logger;
    return logger;
}

std::shared_ptr<spdlog::logger> LogManager::get_core_logger() {
    return get_logger("Core");
}

spdlog::level::level_enum LogManager::parse_level(const std::string& level_str) {
    if (level_str == "trace") return spdlog::level::trace;
    if (level_str == "debug") return spdlog::level::debug;
    if (level_str == "info") return spdlog::level::info;
    if (level_str == "warn") return spdlog::level::warn;
    if (level_str == "err") return spdlog::level::err;
    if (level_str == "critical") return spdlog::level::critical;
    if (level_str == "off") return spdlog::level::off;
    return spdlog::level::info;
}

std::string LogManager::get_current_date_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}

} // namespace cgui
