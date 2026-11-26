#include "nexsolid/Diagnostics.hpp"

#include <BRepBuilderAPI_Copy.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRep_Tool.hxx>
#include <ShapeFix_Shape.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

namespace nexsolid {
namespace {
TopTools_IndexedMapOfShape uniqueShapes(const TopoDS_Shape& shape, TopAbs_ShapeEnum type) {
    TopTools_IndexedMapOfShape result;
    for (TopExp_Explorer explorer(shape, type); explorer.More(); explorer.Next()) {
        result.Add(explorer.Current());
    }
    return result;
}

template<class Getter>
ToleranceRange tolerances(const TopTools_IndexedMapOfShape& shapes, Getter get) {
    ToleranceRange range;
    for (int i = 1; i <= shapes.Extent(); ++i) {
        const double value = get(shapes(i));
        if (!std::isfinite(value) || value < 0.0) {
            throw std::runtime_error("Shape contains an invalid topology tolerance.");
        }
        range.minimum = i == 1 ? value : std::min(range.minimum, value);
        range.maximum = std::max(range.maximum, value);
    }
    return range;
}
} // namespace

ShapeDiagnostics diagnoseShape(const TopoDS_Shape& shape) {
    ShapeDiagnostics report;
    if (shape.IsNull()) return report;
    try {
        const auto solids = uniqueShapes(shape, TopAbs_SOLID);
        const auto shells = uniqueShapes(shape, TopAbs_SHELL);
        const auto faces = uniqueShapes(shape, TopAbs_FACE);
        const auto edges = uniqueShapes(shape, TopAbs_EDGE);
        const auto vertices = uniqueShapes(shape, TopAbs_VERTEX);
        report.valid = BRepCheck_Analyzer(shape).IsValid();
        report.counts = {static_cast<std::size_t>(solids.Extent()),
                         static_cast<std::size_t>(shells.Extent()),
                         static_cast<std::size_t>(faces.Extent()),
                         static_cast<std::size_t>(edges.Extent()),
                         static_cast<std::size_t>(vertices.Extent())};
        report.vertexTolerances = tolerances(vertices, [](const TopoDS_Shape& s) {
            return BRep_Tool::Tolerance(TopoDS::Vertex(s));
        });
        report.edgeTolerances = tolerances(edges, [](const TopoDS_Shape& s) {
            return BRep_Tool::Tolerance(TopoDS::Edge(s));
        });
        report.faceTolerances = tolerances(faces, [](const TopoDS_Shape& s) {
            return BRep_Tool::Tolerance(TopoDS::Face(s));
        });
        return report;
    } catch (const Standard_Failure& error) {
        throw std::runtime_error(std::string("Shape diagnostics failed: ") + error.GetMessageString());
    }
}

TopoDS_Shape repairShape(const TopoDS_Shape& shape, double precision, double maximumTolerance) {
    if (shape.IsNull() || !std::isfinite(precision) || !std::isfinite(maximumTolerance) ||
        precision <= 0.0 || maximumTolerance < precision) {
        throw std::invalid_argument("Shape repair requires a shape and finite 0 < precision <= maximum tolerance.");
    }
    try {
        BRepBuilderAPI_Copy copy(shape, true, false);
        if (!copy.IsDone()) throw std::runtime_error("Shape copy failed before repair.");
        ShapeFix_Shape fixer(copy.Shape());
        fixer.SetPrecision(precision);
        fixer.SetMinTolerance(precision);
        fixer.SetMaxTolerance(maximumTolerance);
        // Perform() reports whether changes were made, not whether the result is valid.
        fixer.Perform();
        const TopoDS_Shape result = fixer.Shape();
        const auto report = diagnoseShape(result);
        if (!report.valid) throw std::runtime_error("Shape repair did not produce a valid shape.");
        if (std::max({report.vertexTolerances.maximum, report.edgeTolerances.maximum,
                      report.faceTolerances.maximum}) > maximumTolerance) {
            throw std::runtime_error("Shape repair exceeded the maximum tolerance.");
        }
        return result;
    } catch (const Standard_Failure& error) {
        throw std::runtime_error(std::string("Shape repair failed: ") + error.GetMessageString());
    }
}

} // namespace nexsolid
