#include "nexsolid/EdgeTreatments.hpp"

#include <BRepBuilderAPI_Copy.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>

#include <cmath>
#include <stdexcept>
#include <string>

namespace nexsolid {
namespace {

double solidVolume(const TopoDS_Shape& shape) {
    GProp_GProps properties;
    BRepGProp::VolumeProperties(shape, properties);
    return properties.Mass();
}

template <typename Builder>
TopoDS_Shape applyTreatment(const TopoDS_Shape& solid,
                           const std::vector<TopoDS_Edge>& edges,
                           double size,
                           const char* operation) {
    const std::string name(operation);
    const std::string failure = name + " failed: ";
    if (!std::isfinite(size) || size <= 0.0) {
        throw std::invalid_argument(name + " size must be finite and positive.");
    }
    if (solid.IsNull() || solid.ShapeType() != TopAbs_SOLID || edges.empty()) {
        throw std::invalid_argument(name + " requires one solid and a nonempty edge selection.");
    }

    try {
        if (!BRepCheck_Analyzer(solid).IsValid()) {
            throw std::invalid_argument(name + " requires a valid solid.");
        }
        const double inputVolume = solidVolume(solid);
        if (!std::isfinite(inputVolume) || inputVolume <= 0.0) {
            throw std::invalid_argument(name + " requires positive finite solid volume.");
        }

        TopTools_IndexedMapOfShape available, selected;
        TopExp::MapShapes(solid, TopAbs_EDGE, available);
        for (const auto& edge : edges) {
            if (edge.IsNull() || !available.Contains(edge)) {
                throw std::invalid_argument(name + " edge does not belong to the input solid.");
            }
            if (selected.Contains(edge)) {
                throw std::invalid_argument(name + " selection contains a duplicate edge.");
            }
            selected.Add(edge);
        }

        BRepBuilderAPI_Copy copy(solid, true, false);
        if (!copy.IsDone() || copy.Shape().IsNull()) {
            throw std::runtime_error(failure + "could not copy the input solid.");
        }
        Builder builder(copy.Shape());
        for (const auto& edge : edges) {
            const auto copiedEdge = TopoDS::Edge(copy.ModifiedShape(edge));
            builder.Add(size, copiedEdge);
            if (builder.Contour(copiedEdge) == 0) {
                throw std::runtime_error(failure + "OCCT did not accept a selected edge.");
            }
        }
        builder.Build();
        if (!builder.IsDone()) {
            throw std::runtime_error(failure + "OCCT did not complete at size " +
                                     std::to_string(size) + ".");
        }

        const TopoDS_Shape result = builder.Shape();
        if (result.IsNull() || !BRepCheck_Analyzer(result).IsValid()) {
            throw std::runtime_error(failure + "OCCT returned an invalid shape.");
        }
        TopTools_IndexedMapOfShape solids;
        TopExp::MapShapes(result, TopAbs_SOLID, solids);
        const double resultVolume = solidVolume(result);
        if (solids.Extent() != 1 || !std::isfinite(resultVolume) || resultVolume <= 0.0) {
            throw std::runtime_error(failure + "result is not one positive-volume solid.");
        }
        return result;
    } catch (const Standard_Failure& error) {
        const char* message = error.GetMessageString();
        throw std::runtime_error(failure + (message ? message : "OCCT exception."));
    }
}

} // namespace

TopoDS_Shape filletEdges(const TopoDS_Shape& solid,
                        const std::vector<TopoDS_Edge>& edges,
                        double radius) {
    return applyTreatment<BRepFilletAPI_MakeFillet>(solid, edges, radius, "fillet");
}

TopoDS_Shape chamferEdges(const TopoDS_Shape& solid,
                         const std::vector<TopoDS_Edge>& edges,
                         double distance) {
    return applyTreatment<BRepFilletAPI_MakeChamfer>(solid, edges, distance, "chamfer");
}

} // namespace nexsolid
