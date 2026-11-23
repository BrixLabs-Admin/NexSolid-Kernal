#include "nexsolid/Kernel.hpp"

#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <GProp_GProps.hxx>

#include <stdexcept>

namespace nexsolid {

TopoDS_Shape makeBox(double x, double y, double z) {
    if (x <= 0.0 || y <= 0.0 || z <= 0.0) {
        throw std::invalid_argument("Box dimensions must be positive.");
    }
    return BRepPrimAPI_MakeBox(x, y, z).Shape();
}

bool isValid(const TopoDS_Shape& shape) {
    return !shape.IsNull() && BRepCheck_Analyzer(shape).IsValid();
}

double volume(const TopoDS_Shape& shape) {
    GProp_GProps properties;
    BRepGProp::VolumeProperties(shape, properties);
    return properties.Mass();
}

} // namespace nexsolid
