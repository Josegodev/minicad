#pragma once

#include <map>
#include <string>
#include <vector>

namespace langcad::geometry {

struct TessellationOptions {
    int segments = 0;
    int rings = 0;
};

struct ShapeDefinition {
    std::string shape_type;
    std::map<std::string, double> dimensions;
    std::string units = "unitless";
    TessellationOptions tessellation;
    bool centered = true;
};

struct CadOperationProposal {
    std::string operation;
    ShapeDefinition shape_definition;
};

struct ShapeValidationResult {
    bool valid = false;
    std::vector<std::string> errors;
    ShapeDefinition normalized_definition;
};

} // namespace langcad::geometry
