#pragma once

#include <TopoDS_Shape.hxx>

namespace nexsolid {

// Creates an axis-aligned box with the given positive dimensions.
TopoDS_Shape makeBox(double x, double y, double z);

// Returns true when the shape is non-null and passes OCCT's BRep validity check.
bool isValid(const TopoDS_Shape& shape);

// Calculates the shape volume in cubic model units.
double volume(const TopoDS_Shape& shape);

} // namespace nexsolid
