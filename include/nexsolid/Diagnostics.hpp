#pragma once

#include <TopoDS_Shape.hxx>
#include <cstddef>

namespace nexsolid {

struct TopologyCounts {
    std::size_t solids = 0, shells = 0, faces = 0, edges = 0, vertices = 0;
};

struct ToleranceRange {
    // Both values are zero when the corresponding topology count is zero.
    double minimum = 0.0;
    double maximum = 0.0;
};

struct ShapeDiagnostics {
    bool valid = false;
    TopologyCounts counts;
    ToleranceRange vertexTolerances, edgeTolerances, faceTolerances;
};

// Counts unique sub-shapes, including the root when it has the requested type.
// Null input produces an invalid report with zero counts/ranges.
ShapeDiagnostics diagnoseShape(const TopoDS_Shape& shape);

// Repair a deep copy, leaving the input untouched. No change is also success.
// Reject null input, invalid tolerance settings, invalid results, or results
// exceeding maximumTolerance. Not every invalid shape can be repaired.
TopoDS_Shape repairShape(const TopoDS_Shape& shape,
                         double precision = 1e-7,
                         double maximumTolerance = 1e-3);

} // namespace nexsolid
