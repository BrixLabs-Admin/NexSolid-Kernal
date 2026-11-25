#include "nexsolid/Kernel.hpp"

#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>

#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>

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
