#pragma once

#include "langcad/geometry/Shape3D.hpp"
#include "langcad/geometry/ShapeDefinition.hpp"

#include <vector>

namespace langcad::geometry {

class FacetedShape final : public Shape3D {
public:
    FacetedShape(
        std::vector<core::Vec3> vertices,
        std::vector<core::Edge> edges,
        std::vector<FaceDefinition> faces
    );

    std::string name() const override;
    core::Mesh toMesh() const override;

private:
    std::vector<core::Vec3> vertices_;
    std::vector<core::Edge> edges_;
    std::vector<FaceDefinition> faces_;
};

} // namespace langcad::geometry
