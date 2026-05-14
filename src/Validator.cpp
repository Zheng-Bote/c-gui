/**
 * SPDX-FileComment: Validator implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file Validator.cpp
 * @brief Implementation of JSON validation wrapper.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "Validator.hpp"
#include "LogManager.hpp"
#include <valijson/validator.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/validation_results.hpp>

namespace cgui {

bool Validator::validate(const nlohmann::json& doc, const valijson::Schema& schema, std::vector<ValidationError>& errors) {
    LogManager::get_instance().get_core_logger()->info("Starting document validation");
    valijson::Validator validator;
    valijson::adapters::NlohmannJsonAdapter doc_adapter(doc);
    valijson::ValidationResults results;

    if (!validator.validate(schema, doc_adapter, &results)) {
        valijson::ValidationResults::Error error;
        while (results.popError(error)) {
            std::string path;
            for (const auto& part : error.context) {
                path += "/" + part;
            }
            errors.push_back({path, error.description});
        }
        LogManager::get_instance().get_core_logger()->warn("Document validation failed with {} errors", errors.size());
        return false;
    }

    LogManager::get_instance().get_core_logger()->info("Document validation successful");
    return true;
}

} // namespace cgui
