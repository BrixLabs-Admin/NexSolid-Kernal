#include "nexsolid/StepIO.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void near(double a, double b, const char* message) {
    require(std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= 1e-5, message);
}
class TemporaryDirectory {
public:
    TemporaryDirectory() {
        std::mt19937_64 random(std::random_device{}());
        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto candidate = std::filesystem::temp_directory_path() /
                                   ("nexsolid-step-" + std::to_string(random()));
            std::error_code error;
            if (std::filesystem::create_directory(candidate, error)) {
                path = candidate;
                return;
            }
            if (error) throw std::filesystem::filesystem_error("Create test directory", candidate, error);
        }
        throw std::runtime_error("Could not allocate a unique STEP test directory.");
    }
    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
    std::filesystem::path path;
};
double volume(const TopoDS_Shape& shape) {
    GProp_GProps properties;
    BRepGProp::VolumeProperties(shape, properties);
    return properties.Mass();
}
int faces(const TopoDS_Shape& shape) {
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(shape, TopAbs_FACE, map);
    return map.Extent();
}
std::array<double, 6> bounds(const TopoDS_Shape& shape) {
    Bnd_Box box;
    BRepBndLib::AddOptimal(shape, box, false, false);
    std::array<double, 6> result{};
    box.Get(result[0], result[1], result[2], result[3], result[4], result[5]);
    return result;
}
TopoDS_Shape fixture() {
    TopTools_ListOfShape arguments, tools;
    arguments.Append(BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape());
    tools.Append(BRepPrimAPI_MakeCylinder(
        gp_Ax2(gp_Pnt(5.0, 5.0, 0.0), gp_Dir(0.0, 0.0, 1.0)), 2.0, 10.0).Shape());
    BRepAlgoAPI_Cut cut;
    cut.SetArguments(arguments);
    cut.SetTools(tools);
    cut.Build();
    require(cut.IsDone(), "Fixture Boolean cut failed.");
    return cut.Shape();
}
template<class Function>
void expectFailure(Function operation) {
    try {
        operation();
    } catch (const std::runtime_error& error) {
        require(std::string(error.what()).find("STEP") != std::string::npos,
                "Import/export error must identify STEP.");
        return;
    }
    throw std::runtime_error("Expected a clean STEP failure.");
}
} // namespace

int main() {
    try {
        TemporaryDirectory temporary;
        const auto original = fixture();
        require(BRepCheck_Analyzer(original).IsValid(), "Invalid fixture.");
        require(faces(original) == 7, "Fixture must have exactly seven faces.");
        near(volume(original), 1000.0 - 40.0 * std::acos(-1.0), "Incorrect fixture volume.");
        const auto file = temporary.path / "box with hole.step";
        nexsolid::writeStep(original, file);
        const auto restored = nexsolid::readStep(file);
        require(BRepCheck_Analyzer(restored).IsValid(), "Invalid STEP round-trip result.");
        require(faces(restored) == faces(original), "STEP round-trip changed face count.");
        near(volume(restored), volume(original), "STEP round-trip changed volume.");
        const auto before = bounds(original), after = bounds(restored);
        for (std::size_t i = 0; i < before.size(); ++i) {
            near(after[i], before[i], "STEP round-trip changed bounds.");
        }

        // Preserve a disconnected second solid as well as the cut fixture.
        const auto plainBox = BRepPrimAPI_MakeBox(gp_Pnt(20.0, 0.0, 0.0), 2.0, 3.0, 4.0).Shape();
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        builder.Add(compound, original);
        builder.Add(compound, plainBox);
        nexsolid::writeStep(compound, temporary.path / "compound.step");
        const auto restoredCompound = nexsolid::readStep(temporary.path / "compound.step");
        near(volume(restoredCompound), volume(original) + 24.0,
             "STEP round-trip lost a disconnected solid.");
        require(faces(restoredCompound) == 13, "STEP round-trip changed compound face count.");
        expectFailure([&] { nexsolid::readStep(temporary.path / "missing.step"); });
        expectFailure([&] { nexsolid::readStep(temporary.path); });
        const auto corrupt = temporary.path / "corrupt.step";
        { std::ofstream stream(corrupt); stream << "not a STEP file\n"; }
        expectFailure([&] { nexsolid::readStep(corrupt); });
        const auto empty = temporary.path / "empty.step";
        { std::ofstream stream(empty); }
        expectFailure([&] { nexsolid::readStep(empty); });
        expectFailure([&] { nexsolid::writeStep(original, temporary.path / "missing" / "out.step"); });
        bool rejectedNull = false;
        try { nexsolid::writeStep(TopoDS_Shape(), temporary.path / "null.step"); }
        catch (const std::invalid_argument&) { rejectedNull = true; }
        require(rejectedNull, "Null shape was accepted for STEP export.");
        std::cout << "STEP round-trip and failure-recovery checks passed.\n";
        return 0;
    } catch (const Standard_Failure& error) {
        std::cerr << "Unexpected OCCT failure: " << error.GetMessageString() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
    return 1;
}
