#pragma once

#include <TopoDS_Shape.hxx>

namespace nexsolid {

// Creates an axis-aligned box with the given positive dimensions.
TopoDS_Shape makeBox(double x, double y, double z);

// Creates a 10x10x10 box with a radius-2 through-hole along Z at (5, 5).
// Throws std::runtime_error if BRepAlgoAPI_Cut::IsDone() is false.
TopoDS_Shape makeBoxWithCylindricalHole();

// Same box with a configurable through-cylinder: 0 < radius < 5, height >= 10.
// Non-finite or out-of-range dimensions throw std::invalid_argument.
TopoDS_Shape makeBoxWithCylindricalHole(double radius, double height);

// Returns true when the shape is non-null and passes OCCT's BRep validity check.
bool isValid(const TopoDS_Shape& shape);

// Calculates the shape volume in cubic model units.
double volume(const TopoDS_Shape& shape);

} // namespace nexsolid
