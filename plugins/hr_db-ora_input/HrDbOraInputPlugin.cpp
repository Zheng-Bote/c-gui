/**
 * SPDX-FileComment: Oracle DB Input Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrDbOraInputPlugin.cpp
 * @brief Implementation of Oracle DB input plugin using OCILIB.
 * @version 1.2.0
 * @date 2026-05-23
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "HrDbOraInputPlugin.hpp"
#include <map>
#include <sstream>
#include <iostream>

namespace cgui {

/**
 * @brief Helper to parse connection string parts.
 */
static std::map<std::string, std::string> parse_conn_params(const std::string& conn_str) {
    std::map<std::string, std::string> result;
    std::stringstream ss(conn_str);
    std::string segment;

    while (std::getline(ss, segment, ',')) {
        size_t colon_pos = segment.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = segment.substr(0, colon_pos);
            std::string value = segment.substr(colon_pos + 1);

            // Trim whitespace
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" "));
                s.erase(s.find_last_not_of(" ") + 1);
            };
            trim(key);
            trim(value);

            result[key] = value;
        }
    }
    return result;
}

bool HrDbOraInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("connection_string") || config["connection_string"].get<std::string>().empty()) {
        std::cerr << "Oracle Plugin Error: Connection string is empty or missing.\n";
        return false;
    }

    m_connection_string = config["connection_string"].get<std::string>();

    try {
        auto params = parse_conn_params(m_connection_string);
        
        if (params.find("host") == params.end() || params.find("user") == params.end() || params.find("password") == params.end()) {
            std::cerr << "Oracle Plugin Error: Missing required params (host, user, password) in configuration.\n";
            return false;
        }

        std::string host = params["host"];
        std::string port = params.count("port") ? params["port"] : "1521";
        std::string user = params["user"];
        std::string password = params["password"];
        std::string service = params.count("service") ? params["service"] : "";

        // SOCI Oracle format: "service=my_service user=my_user password=my_password"
        // Or using EZConnect: "service=//host:port/service_name user=... password=..."
        std::string conn_info;
        if (!host.empty()) {
            conn_info = "service=//" + host + ":" + port + (service.empty() ? "" : "/" + service);
        } else {
            conn_info = "service=" + service;
        }
        conn_info += " user=" + user + " password=" + password;

        std::cout << "Oracle Plugin: Attempting connection to " << host << " via SOCI oracle backend...\n";
        
        // Note: Under Windows, this requires soci_oracle.dll to be in the PATH or same directory
        // and the Oracle Instant Client (oci.dll) to be in the PATH.
        m_sql = std::make_unique<soci::session>("oracle", conn_info);
        m_initialized = true;
        std::cout << "Oracle Plugin: Successfully connected to database.\n";
    } catch (const soci::soci_error& e) {
        std::cerr << "Oracle Plugin Error (SOCI): " << e.what() << "\n";
        m_initialized = false;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Oracle Plugin Error (Std): " << e.what() << "\n";
        m_initialized = false;
        return false;
    }

    return true;
}

std::vector<nlohmann::json> HrDbOraInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    
    if (!m_initialized || !m_sql) {
        std::cerr << "Oracle Plugin: fetch_batch called but plugin not initialized.\n";
        return batch;
    }

    try {
        // Example query for HR data
        std::string query = "SELECT employee_id, first_name, last_name, email FROM employees WHERE ROWNUM <= :max_rec";

        std::cout << "Oracle Plugin: Executing query via SOCI...\n";
        
        soci::rowset<soci::row> rs = (m_sql->prepare << query, soci::use(max_records));

        for (auto it = rs.begin(); it != rs.end(); ++it) {
            const soci::row& row = *it;
            nlohmann::json j_row;
            
            for (std::size_t i = 0; i < row.size(); ++i) {
                const soci::column_properties& props = row.get_properties(i);
                std::string name = props.get_name();
                
                switch (props.get_data_type()) {
                    case soci::dt_string: j_row[name] = row.get<std::string>(i); break;
                    case soci::dt_double: j_row[name] = row.get<double>(i); break;
                    case soci::dt_integer: j_row[name] = row.get<int>(i); break;
                    case soci::dt_long_long: j_row[name] = row.get<long long>(i); break;
                    case soci::dt_date: {
                        std::tm t = row.get<std::tm>(i);
                        char buf[64];
                        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
                        j_row[name] = std::string(buf);
                        break;
                    }
                    default: j_row[name] = row.get<std::string>(i); break;
                }
            }
            batch.push_back(j_row);
        }
        
        std::cout << "Oracle Plugin: Successfully fetched " << batch.size() << " records.\n";
    } catch (const soci::soci_error& e) {
        std::cerr << "Oracle Plugin: Query execution failed: " << e.what() << "\n";
    }

    return batch;
}

void HrDbOraInputPlugin::shutdown() {
    if (m_sql) {
        m_sql->close();
        m_sql.reset();
    }
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::HrDbOraInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
