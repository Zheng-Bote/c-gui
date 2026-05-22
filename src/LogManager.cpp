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
#include <spdlog/sinks/base_sink.h>
#include <spdlog/pattern_formatter.h>
#include <sodium.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>

namespace cgui {

namespace {
    LogManager::LogCallback g_callback = nullptr;

    /**
     * @brief Custom sink that signs log entries using Ed25519.
     */
    template<typename Mutex>
    class audit_signing_sink : public spdlog::sinks::base_sink<Mutex> {
    public:
        audit_signing_sink(std::shared_ptr<spdlog::sinks::sink> target_sink, const std::vector<unsigned char>& private_key)
            : m_target_sink(std::move(target_sink)), m_private_key(private_key) {}

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            if (m_private_key.empty()) {
                m_target_sink->log(msg);
                return;
            }

            // 1. Format the message
            spdlog::memory_buf_t formatted;
            this->formatter_->format(msg, formatted);
            std::string text = fmt::to_string(formatted);

            // Strip trailing newline if present for signing consistency
            if (!text.empty() && text.back() == '\n') text.pop_back();
            if (!text.empty() && text.back() == '\r') text.pop_back();

            // 2. Sign
            unsigned char sig[crypto_sign_BYTES];
            crypto_sign_detached(sig, nullptr, 
                                 reinterpret_cast<const unsigned char*>(text.c_str()), text.length(), 
                                 m_private_key.data());

            // 3. Convert signature to hex
            char sig_hex[crypto_sign_BYTES * 2 + 1];
            sodium_bin2hex(sig_hex, sizeof(sig_hex), sig, sizeof(sig));

            // 4. Create new message with signature appended
            std::string signed_text = text + " | sig: " + std::string(sig_hex) + "\n";
            
            spdlog::details::log_msg signed_msg(msg.time, msg.source, msg.logger_name, msg.level, signed_text);
            m_target_sink->log(signed_msg);
        }

        void flush_() override {
            m_target_sink->flush();
        }

    private:
        std::shared_ptr<spdlog::sinks::sink> m_target_sink;
        std::vector<unsigned char> m_private_key;
    };

    using audit_signing_sink_mt = audit_signing_sink<std::mutex>;
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

std::shared_ptr<spdlog::logger> LogManager::get_audit_logger(const std::string& topic) {
    std::string date_str = get_current_date_str();
    std::string logger_name = date_str + "_" + topic + "_audit";

    auto it = m_audit_loggers.find(logger_name);
    if (it != m_audit_loggers.end()) {
        return it->second;
    }

    // Audit logs go to a separate file
    std::string filename = (std::filesystem::path(m_log_path) / (logger_name + ".log")).string();
    
    // File sink for the actual storage
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, 1024 * 1024 * 10, 5);
    file_sink->set_pattern("%v"); // Underlying sink should only write the formatted and signed payload
    
    // Wrap it in our signing sink
    auto signing_sink = std::make_shared<audit_signing_sink_mt>(file_sink, m_signing_key);
    
    // We also want audit logs in the console and GUI (without the signature there to keep it clean)
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    std::vector<spdlog::sink_ptr> sinks {signing_sink, console_sink};

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
    logger->flush_on(spdlog::level::trace);

    m_audit_loggers[logger_name] = logger;
    return logger;
}

void LogManager::set_signing_key(const std::string& hex_key) {
    if (hex_key.length() != crypto_sign_SECRETKEYBYTES * 2) {
        get_core_logger()->error("Invalid Ed25519 secret key length. Expected {} hex characters.", crypto_sign_SECRETKEYBYTES * 2);
        return;
    }

    m_signing_key_hex = hex_key;
    m_signing_key.resize(crypto_sign_SECRETKEYBYTES);
    
    if (sodium_hex2bin(m_signing_key.data(), m_signing_key.size(), 
                       hex_key.c_str(), hex_key.length(), 
                       nullptr, nullptr, nullptr) != 0) {
        get_core_logger()->error("Failed to parse Ed25519 secret key from hex.");
        m_signing_key.clear();
        return;
    }

    get_core_logger()->info("Audit log signing key configured successfully.");
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
