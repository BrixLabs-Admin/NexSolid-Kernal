#include "nexsolid/Properties.hpp"

#include <BRepBndLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <GProp_PrincipalProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace nexsolid {
namespace {
void requireFinite(double value) {
    if (!std::isfinite(value)) throw std::runtime_error("Non-finite geometric property.");
}
void requireFinite(const gp_Pnt& point) {
    requireFinite(point.X());
    requireFinite(point.Y());
    requireFinite(point.Z());
}
void requireValid(const TopoDS_Shape& shape) {
    if (shape.IsNull() || !BRepCheck_Analyzer(shape).IsValid()) {
        throw std::invalid_argument("Geometric properties require a non-null valid shape.");
    }
}
IndexedSubShape describe(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, double grid) {
    IndexedSubShape item;
    item.shape = shape;
    Bnd_Box box;
    BRepBndLib::AddOptimal(shape, box, false, false);
    if (box.IsVoid() || box.IsOpen()) {
        throw std::invalid_argument("Topology indexing requires finite bounded sub-shapes.");
    }
    box.Get(item.bounds[0], item.bounds[1], item.bounds[2],
            item.bounds[3], item.bounds[4], item.bounds[5]);
    for (double coordinate : item.bounds) requireFinite(coordinate);
    if (type == TopAbs_VERTEX) {
        item.centroid = BRep_Tool::Pnt(TopoDS::Vertex(shape));
    } else {
        GProp_GProps properties;
        if (type == TopAbs_SOLID) {
            BRepGProp::VolumeProperties(shape, properties, true, true);
        } else if (type == TopAbs_EDGE) {
            BRepGProp::LinearProperties(shape, properties, true);
        } else {
            BRepGProp::SurfaceProperties(shape, properties, true);
        }
        item.measure = std::abs(properties.Mass());
        requireFinite(item.measure);
        if (item.measure > 0.0) {
            item.centroid = properties.CentreOfMass();
        } else {
            item.centroid = gp_Pnt(item.bounds[0] / 2.0 + item.bounds[3] / 2.0,
                                   item.bounds[1] / 2.0 + item.bounds[4] / 2.0,
                                   item.bounds[2] / 2.0 + item.bounds[5] / 2.0);
        }
    }
    requireFinite(item.centroid);
    item.geometricKey = {item.centroid.X(), item.centroid.Y(), item.centroid.Z(), item.measure,
                         item.bounds[0], item.bounds[1], item.bounds[2],
                         item.bounds[3], item.bounds[4], item.bounds[5]};
    for (double& value : item.geometricKey) {
        value = std::round(value / grid);
        requireFinite(value);
    }
    return item;
}

std::vector<IndexedSubShape> index(const TopoDS_Shape& shape, TopAbs_ShapeEnum type,
                                  double grid, bool& ambiguous) {
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(shape, type, map);
    std::vector<IndexedSubShape> items;
    items.reserve(static_cast<std::size_t>(map.Extent()));
    for (int i = 1; i <= map.Extent(); ++i) items.push_back(describe(map(i), type, grid));
    std::stable_sort(items.begin(), items.end(), [](const auto& left, const auto& right) {
        return left.geometricKey < right.geometricKey;
    });
    for (std::size_t i = 1; i < items.size(); ++i) {
        if (items[i - 1].geometricKey == items[i].geometricKey) ambiguous = true;
    }
    return items;
}
} // namespace

MassProperties massProperties(const TopoDS_Shape& shape) {
    try {
        requireValid(shape);
        TopTools_IndexedMapOfShape solids;
        TopExp::MapShapes(shape, TopAbs_SOLID, solids);
        if (solids.IsEmpty()) throw std::invalid_argument("Mass properties require solid geometry.");
        GProp_GProps volume, surface, linear;
        BRepGProp::VolumeProperties(shape, volume, true, true);
        BRepGProp::SurfaceProperties(shape, surface, true);
        BRepGProp::LinearProperties(shape, linear, true);
        MassProperties result;
        result.volume = volume.Mass();
        result.surfaceArea = surface.Mass();
        result.edgeLength = linear.Mass();
        result.centerOfMass = volume.CentreOfMass();
        volume.PrincipalProperties().Moments(result.principalMoments[0],
                                              result.principalMoments[1],
                                              result.principalMoments[2]);
        requireFinite(result.volume);
        requireFinite(result.surfaceArea);
        requireFinite(result.edgeLength);
        requireFinite(result.centerOfMass);
        if (result.volume <= 0.0 || result.surfaceArea < 0.0 || result.edgeLength < 0.0) {
            throw std::invalid_argument("Mass properties require positive oriented solid volume.");
        }
        for (double moment : result.principalMoments) {
            requireFinite(moment);
            if (moment < 0.0) throw std::runtime_error("Negative principal moment of inertia.");
        }
        std::sort(result.principalMoments.begin(), result.principalMoments.end());
        return result;
    } catch (const Standard_Failure& error) {
        throw std::runtime_error(std::string("Mass properties failed: ") + error.GetMessageString());
    }
}

TopologyIndex indexTopology(const TopoDS_Shape& shape, double keyTolerance) {
    if (!std::isfinite(keyTolerance) || keyTolerance <= 0.0) {
        throw std::invalid_argument("Topology key tolerance must be finite and positive.");
    }
    try {
        requireValid(shape);
        TopologyIndex result;
        result.solids = index(shape, TopAbs_SOLID, keyTolerance, result.hasAmbiguousKeys);
        result.shells = index(shape, TopAbs_SHELL, keyTolerance, result.hasAmbiguousKeys);
        result.faces = index(shape, TopAbs_FACE, keyTolerance, result.hasAmbiguousKeys);
        result.edges = index(shape, TopAbs_EDGE, keyTolerance, result.hasAmbiguousKeys);
        result.vertices = index(shape, TopAbs_VERTEX, keyTolerance, result.hasAmbiguousKeys);
        return result;
    } catch (const Standard_Failure& error) {
        throw std::runtime_error(std::string("Topology indexing failed: ") + error.GetMessageString());
    }
}

} // namespace nexsolid
