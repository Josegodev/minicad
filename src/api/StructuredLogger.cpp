#include "langcad/api/StructuredLogger.hpp"

#include <iostream>
#include <mutex>

namespace langcad::api {

void StructuredLogger::logRequest(
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
) const {
    static std::mutex output_mutex;

    nlohmann::json entry = {
        {"trace_id", trace_id},
        {"endpoint", endpoint},
        {"model", model},
        {"shape_type", shape_type},
        {"dimensions", dimensions},
        {"validation_ok", validation_ok},
        {"render_started", render_started},
        {"error_type", error_type},
        {"latency_ms", latency_ms},
        {"status", status}
    };

    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << entry.dump() << '\n';
}

} // namespace langcad::api
