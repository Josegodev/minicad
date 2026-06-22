#pragma once

#include "langcad/geometry/ShapeDefinition.hpp"

#include <nlohmann/json.hpp>

#include <string>

namespace langcad::api {

class StructuredLogger {
public:
    void logRequest(
        const std::string& trace_id,
        const std::string& endpoint,
        const std::string& model,
        const std::string& shape_type,
        const nlohmann::json& dimensions,
        bool validation_ok,
        bool render_started,
        const std::string& error_type,
        long long latency_ms,
        const std::string& status
    ) const;
};

} // namespace langcad::api
