#include "nexsolid/Kernel.hpp"

#include <BRepCheck_Analyzer.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <stdexcept>

namespace {
int uniqueCount(const TopoDS_Shape& shape, TopAbs_ShapeEnum type) {
    TopTools_IndexedMapOfShape shapes;
    for (TopExp_Explorer it(shape, type); it.More(); it.Next()) {
        shapes.Add(it.Current());
    }
    return shapes.Extent();
}

void expectInvalidDimensions(double radius, double height) {
    try {
        (void)nexsolid::makeBoxWithCylindricalHole(radius, height);
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("Invalid cylinder dimensions were accepted");
}
} // namespace

int main() {
    try {
        // The helper throws unless BRepAlgoAPI_Cut::IsDone() is true.
        const auto shape = nexsolid::makeBoxWithCylindricalHole();
        if (shape.IsNull()) {
            std::cerr << "Boolean cut returned a null shape\n";
            return 1;
        }
        if (!BRepCheck_Analyzer(shape).IsValid()) {
            std::cerr << "Boolean cut returned an invalid shape\n";
            return 1;
        }

        // Unique edges include the cylindrical seam; explorer occurrences repeat edges.
        if (uniqueCount(shape, TopAbs_FACE) != 7 || uniqueCount(shape, TopAbs_EDGE) != 15) {
            throw std::runtime_error("Expected exactly 7 faces and 15 unique edges");
        }
        Bnd_Box bounds;
        BRepBndLib::AddOptimal(shape, bounds, false, false);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        bounds.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        for (double lower : {xmin, ymin, zmin}) {
            if (!std::isfinite(lower) || std::abs(lower) > 1e-7) {
                throw std::runtime_error("Unexpected minimum box bound");
            }
        }
        for (double upper : {xmax, ymax, zmax}) {
            if (!std::isfinite(upper) || std::abs(upper - 10.0) > 1e-7) {
                throw std::runtime_error("Unexpected maximum box bound");
            }
        }

        const double infinity = std::numeric_limits<double>::infinity();
        const double nan = std::numeric_limits<double>::quiet_NaN();
        for (double radius : {0.0, -2.0, 5.0, infinity, nan}) {
            expectInvalidDimensions(radius, 10.0);
        }
        for (double height : {0.0, -10.0, 9.0, infinity, nan}) {
            expectInvalidDimensions(2.0, height);
        }
        const auto extendedTool = nexsolid::makeBoxWithCylindricalHole(2.0, 20.0);
        if (!nexsolid::isValid(extendedTool) ||
            std::abs(nexsolid::volume(extendedTool) - nexsolid::volume(shape)) > 1e-5) {
            throw std::runtime_error("A longer through-cylinder changed the result");
        }

        GProp_GProps properties;
        BRepGProp::VolumeProperties(shape, properties);
        const double actualVolume = properties.Mass();
        const double expectedVolume = 1000.0 - std::acos(-1.0) * 4.0 * 10.0;
        constexpr double tolerance = 1e-5;
        if (!std::isfinite(actualVolume) ||
            std::abs(actualVolume - expectedVolume) > tolerance) {
            std::cerr << std::setprecision(17)
                      << "Boolean cut volume: expected " << expectedVolume
                      << ", got " << actualVolume
                      << " (tolerance " << tolerance << ")\n";
            return 1;
        }

        std::cout << "Boolean cut regression passed: valid shape, volume = "
                  << std::setprecision(17) << actualVolume << '\n';
        return 0;
    } catch (const Standard_Failure& error) {
        std::cerr << "OCCT failure: " << error.GetMessageString() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Boolean cut failed: " << error.what() << '\n';
        return 1;
    }
}
