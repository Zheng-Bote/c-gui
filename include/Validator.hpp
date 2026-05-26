/**
 * SPDX-FileComment: Validator for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file Validator.hpp
 * @brief Wrapper for valijson to validate JSON documents.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_VALIDATOR_HPP
#define C_GUI_VALIDATOR_HPP

#include <nlohmann/json.hpp>
#include <valijson/schema.hpp>
#include <string>
#include <vector>
#include <memory>
#include "common.hpp"

namespace cgui {

struct CGUI_API ValidationError {
    std::string path;
    std::string message;
};

/**
 * @brief Validates JSON documents against a schema.
 */
class CGUI_API Validator {
public:
    /**
     * @brief Validate a JSON document against a schema.
     * @param doc The JSON document to validate.
     * @param schema The schema to validate against.
     * @param errors Output vector for validation errors.
     * @return true if valid, false otherwise.
     */
    static bool validate(const nlohmann::json& doc, const valijson::Schema& schema, std::vector<ValidationError>& errors);
};

} // namespace cgui

#endif // C_GUI_VALIDATOR_HPP
