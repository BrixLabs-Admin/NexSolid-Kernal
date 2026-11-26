#pragma once

#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <vector>

namespace nexsolid {

// Apply a constant-radius fillet or a symmetric chamfer to selected edges of
// one valid, positive-volume solid. Sizes are positive finite model units.
// Selection must be nonempty, unique, and drawn from the input's own topology.
// OCCT may propagate a selected edge along its tangent contour.
//
// Operations use a deep geometry/topology copy, preserving the input even on
// failure. Invalid arguments throw std::invalid_argument; OCCT failures and
// invalid results throw std::runtime_error with the operation name.
// Success returns a valid shape containing one positive-volume solid.
// This does not guarantee success for arbitrary radii or difficult geometry.
TopoDS_Shape filletEdges(const TopoDS_Shape& solid,
                        const std::vector<TopoDS_Edge>& edges,
                        double radius);

TopoDS_Shape chamferEdges(const TopoDS_Shape& solid,
                         const std::vector<TopoDS_Edge>& edges,
                         double distance);

} // namespace nexsolid
