/**
 * SPDX-FileComment: Unit tests for Validator
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file tests/test_validator.cpp
 * @brief Validator unit tests
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>
#include "Validator.hpp"
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>
#include <valijson/schema_parser.hpp>

using namespace cgui;

TEST_CASE("Validator validates JSON against schema", "[Validator]") {
    nlohmann::json schema_json = R"({
        "$schema": "http://json-schema.org/draft-07/schema#",
        "type": "object",
        "properties": {
            "name": {"type": "string"},
            "age": {"type": "integer", "minimum": 0}
        },
        "required": ["name", "age"]
    })"_json;

    valijson::Schema schema;
    valijson::SchemaParser parser;
    valijson::adapters::NlohmannJsonAdapter schema_adapter(schema_json);
    parser.populateSchema(schema_adapter, schema);

    SECTION("Valid JSON") {
        nlohmann::json valid_doc = R"({
            "name": "John Doe",
            "age": 30
        })"_json;

        std::vector<ValidationError> errors;
        bool result = Validator::validate(valid_doc, schema, errors);
        
        REQUIRE(result == true);
        REQUIRE(errors.empty());
    }

    SECTION("Invalid JSON - missing required field") {
        nlohmann::json invalid_doc = R"({
            "name": "John Doe"
        })"_json;

        std::vector<ValidationError> errors;
        bool result = Validator::validate(invalid_doc, schema, errors);
        
        REQUIRE(result == false);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Invalid JSON - wrong type") {
        nlohmann::json invalid_doc = R"({
            "name": "John Doe",
            "age": "thirty"
        })"_json;

        std::vector<ValidationError> errors;
        bool result = Validator::validate(invalid_doc, schema, errors);
        
        REQUIRE(result == false);
        REQUIRE_FALSE(errors.empty());
    }
}
