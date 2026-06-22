#pragma once

#include "langcad/geometry/ShapeDefinition.hpp"

#include <string>

namespace langcad::llm {

struct OllamaConfig {
    std::string base_url = "http://localhost:11434";
    std::string model = "qwen2.5-coder:7b";
};

struct OllamaShapeResponse {
    bool ok = false;
    std::string error;
    geometry::ShapeDefinition shape_definition;
    geometry::ShapeValidationResult validation;
    std::string model;
    long long latency_ms = 0;
};

class OllamaClient {
public:
    explicit OllamaClient(OllamaConfig config);

    OllamaShapeResponse generateShapeDefinition(const std::string& prompt) const;

private:
    OllamaConfig config_;
};

OllamaConfig ollamaConfigFromEnvironment();

} // namespace langcad::llm
