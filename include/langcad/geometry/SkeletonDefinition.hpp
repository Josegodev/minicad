#pragma once

#include "langcad/core/Vec3.hpp"

#include <string>
#include <vector>

namespace langcad::geometry {

enum class CoordinatePlane {
    XY,
    YZ,
    XZ
};

enum class PlaneDefinitionType {
    CoordinateOffset,
    OffsetFromPlane,
    NormalAxisFromOrigin,
    NormalAxisFromPlane
};

struct AxisDefinition {
    std::string id;
    core::Vec3 direction;
};

struct PlaneDefinition {
    std::string id;
    PlaneDefinitionType type = PlaneDefinitionType::CoordinateOffset;
    CoordinatePlane coordinate_plane = CoordinatePlane::XY;
    std::string reference_plane_id;
    std::string normal_axis_id;
    double distance = 0.0;
};

struct SkeletonDefinition {
    std::vector<AxisDefinition> axes;
    std::vector<PlaneDefinition> planes;
};

struct ResolvedAxis {
    std::string id;
    core::Vec3 direction;
};

struct ResolvedPlane {
    std::string id;
    core::Vec3 normal;
    double offset = 0.0;
};

struct SkeletonResolutionResult {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<ResolvedAxis> axes;
    std::vector<ResolvedPlane> planes;
};

} // namespace langcad::geometry
