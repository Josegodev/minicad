#pragma once

#include "langcad/core/Mesh.hpp"
#include "langcad/geometry/ShapeDefinition.hpp"
#include "langcad/geometry/ShapeCatalog.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace langcad::api {

struct ParsedShapeDefinition {
    geometry::ShapeDefinition definition;
    std::vector<std::string> errors;
};

ParsedShapeDefinition parseShapeDefinitionJson(const nlohmann::json& input);

nlohmann::json shapeDefinitionToJson(const geometry::ShapeDefinition& definition);
nlohmann::json validationToJson(const geometry::ShapeValidationResult& validation);
nlohmann::json meshToJson(const core::Mesh& mesh);
nlohmann::json supportedShapesToJson();
nlohmann::json shapeDefinitionSchemaJson();

} // namespace langcad::api
