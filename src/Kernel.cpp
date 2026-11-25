#include "nexsolid/Kernel.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GProp_GProps.hxx>
#include <TopTools_ListOfShape.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <cmath>
#include <stdexcept>

namespace nexsolid {

TopoDS_Shape makeBox(double x, double y, double z) {
    if (x <= 0.0 || y <= 0.0 || z <= 0.0) {
        throw std::invalid_argument("Box dimensions must be positive.");
    }
    return BRepPrimAPI_MakeBox(x, y, z).Shape();
}

TopoDS_Shape makeBoxWithCylindricalHole() {
    return makeBoxWithCylindricalHole(2.0, 10.0);
}

TopoDS_Shape makeBoxWithCylindricalHole(double radius, double height) {
    if (!std::isfinite(radius) || !std::isfinite(height) ||
        radius <= 0.0 || radius >= 5.0 || height < 10.0) {
        throw std::invalid_argument(
            "Through-cylinder dimensions require finite 0 < radius < 5 and height >= 10.");
    }
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
    const gp_Ax2 axis(gp_Pnt(5.0, 5.0, 0.0), gp_Dir(0.0, 0.0, 1.0));
    const TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(axis, radius, height).Shape();

    TopTools_ListOfShape arguments;
    arguments.Append(box);
    TopTools_ListOfShape tools;
    tools.Append(cylinder);

    BRepAlgoAPI_Cut cut;
    cut.SetArguments(arguments);
    cut.SetTools(tools);
    cut.Build();
    if (!cut.IsDone()) {
        throw std::runtime_error("Box-cylinder Boolean cut failed.");
    }
    return cut.Shape();
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
