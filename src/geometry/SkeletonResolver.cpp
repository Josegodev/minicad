#include "langcad/geometry/SkeletonResolver.hpp"

#include <map>
#include <set>

namespace langcad::geometry {

namespace {

core::Vec3 coordinateNormal(CoordinatePlane plane) {
    switch (plane) {
    case CoordinatePlane::XY:
        return core::Vec3(0.0, 0.0, 1.0);
    case CoordinatePlane::YZ:
        return core::Vec3(1.0, 0.0, 0.0);
    case CoordinatePlane::XZ:
        return core::Vec3(0.0, 1.0, 0.0);
    }

    return core::Vec3(0.0, 0.0, 1.0);
}

bool hasId(const std::string& id) {
    return !id.empty();
}

const ResolvedAxis* findAxis(const std::vector<ResolvedAxis>& axes, const std::string& id) {
    for (const auto& axis : axes) {
        if (axis.id == id) {
            return &axis;
        }
    }

    return nullptr;
}

const ResolvedPlane* findPlane(const std::vector<ResolvedPlane>& planes, const std::string& id) {
    for (const auto& plane : planes) {
        if (plane.id == id) {
            return &plane;
        }
    }

    return nullptr;
}

bool planeCanResolve(
    const PlaneDefinition& plane,
    const std::vector<ResolvedAxis>& axes,
    const std::vector<ResolvedPlane>& planes,
    std::vector<std::string>& errors
) {
    switch (plane.type) {
    case PlaneDefinitionType::CoordinateOffset:
        return true;
    case PlaneDefinitionType::OffsetFromPlane:
        if (!hasId(plane.reference_plane_id)) {
            errors.push_back("Plane '" + plane.id + "' requires reference_plane_id");
            return false;
        }
        return findPlane(planes, plane.reference_plane_id) != nullptr;
    case PlaneDefinitionType::NormalAxisFromOrigin:
        if (!hasId(plane.normal_axis_id)) {
            errors.push_back("Plane '" + plane.id + "' requires normal_axis_id");
            return false;
        }
        return findAxis(axes, plane.normal_axis_id) != nullptr;
    case PlaneDefinitionType::NormalAxisFromPlane:
        if (!hasId(plane.normal_axis_id)) {
            errors.push_back("Plane '" + plane.id + "' requires normal_axis_id");
            return false;
        }
        if (!hasId(plane.reference_plane_id)) {
            errors.push_back("Plane '" + plane.id + "' requires reference_plane_id");
            return false;
        }
        return findAxis(axes, plane.normal_axis_id) != nullptr
            && findPlane(planes, plane.reference_plane_id) != nullptr;
    }

    return false;
}

ResolvedPlane resolvePlane(
    const PlaneDefinition& plane,
    const std::vector<ResolvedAxis>& axes,
    const std::vector<ResolvedPlane>& planes
) {
    ResolvedPlane resolved;
    resolved.id = plane.id;

    switch (plane.type) {
    case PlaneDefinitionType::CoordinateOffset:
        resolved.normal = coordinateNormal(plane.coordinate_plane);
        resolved.offset = plane.distance;
        break;
    case PlaneDefinitionType::OffsetFromPlane: {
        const ResolvedPlane* reference = findPlane(planes, plane.reference_plane_id);
        resolved.normal = reference->normal;
        resolved.offset = reference->offset + plane.distance;
        break;
    }
    case PlaneDefinitionType::NormalAxisFromOrigin: {
        const ResolvedAxis* axis = findAxis(axes, plane.normal_axis_id);
        resolved.normal = axis->direction;
        resolved.offset = plane.distance;
        break;
    }
    case PlaneDefinitionType::NormalAxisFromPlane: {
        const ResolvedAxis* axis = findAxis(axes, plane.normal_axis_id);
        const ResolvedPlane* reference = findPlane(planes, plane.reference_plane_id);
        core::Vec3 reference_point = reference->normal * reference->offset;
        resolved.normal = axis->direction;
        resolved.offset = axis->direction.dot(reference_point) + plane.distance;
        break;
    }
    }

    return resolved;
}

} // namespace

SkeletonResolutionResult resolveSkeleton(const SkeletonDefinition& definition) {
    SkeletonResolutionResult result;
    std::set<std::string> ids;

    for (const auto& axis : definition.axes) {
        if (!hasId(axis.id)) {
            result.errors.push_back("Axis id must not be empty");
            continue;
        }

        if (!ids.insert("axis:" + axis.id).second) {
            result.errors.push_back("Duplicate axis id '" + axis.id + "'");
            continue;
        }

        core::Vec3 direction = axis.direction.normalized();
        if (direction.norm() == 0.0) {
            result.errors.push_back("Axis '" + axis.id + "' direction must not be zero");
            continue;
        }

        result.axes.push_back({axis.id, direction});
    }

    std::vector<PlaneDefinition> pending_planes;
    for (const auto& plane : definition.planes) {
        if (!hasId(plane.id)) {
            result.errors.push_back("Plane id must not be empty");
            continue;
        }

        if (!ids.insert("plane:" + plane.id).second) {
            result.errors.push_back("Duplicate plane id '" + plane.id + "'");
            continue;
        }

        pending_planes.push_back(plane);
    }

    bool made_progress = true;
    while (made_progress && !pending_planes.empty()) {
        made_progress = false;

        for (auto it = pending_planes.begin(); it != pending_planes.end();) {
            std::vector<std::string> missing_required_fields;
            if (planeCanResolve(*it, result.axes, result.planes, missing_required_fields)) {
                result.planes.push_back(resolvePlane(*it, result.axes, result.planes));
                it = pending_planes.erase(it);
                made_progress = true;
            } else if (!missing_required_fields.empty()) {
                result.errors.insert(result.errors.end(), missing_required_fields.begin(), missing_required_fields.end());
                it = pending_planes.erase(it);
                made_progress = true;
            } else {
                ++it;
            }
        }
    }

    for (const auto& plane : pending_planes) {
        result.errors.push_back("Plane '" + plane.id + "' has unresolved or cyclic references");
    }

    result.valid = result.errors.empty();
    return result;
}

} // namespace langcad::geometry
