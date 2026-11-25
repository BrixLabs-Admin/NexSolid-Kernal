#include "nexsolid/Diagnostics.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRep_Builder.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void near(double value, double expected, double tolerance = 1e-6) {
    require(std::isfinite(value) && std::abs(value - expected) <= tolerance, "Unexpected numeric result.");
}
double volume(const TopoDS_Shape& shape) {
    GProp_GProps properties;
    BRepGProp::VolumeProperties(shape, properties);
    return properties.Mass();
}
void checkCounts(const nexsolid::ShapeDiagnostics& d, std::size_t faces,
                 std::size_t edges, std::size_t vertices) {
    require(d.valid, "Expected valid geometry.");
    require(d.counts.solids == 1 && d.counts.shells == 1 &&
            d.counts.faces == faces && d.counts.edges == edges && d.counts.vertices == vertices,
            "Incorrect unique topology counts.");
}
void checkRange(const nexsolid::ToleranceRange& range) {
    require(range.minimum > 0.0 && range.maximum >= range.minimum &&
            range.maximum <= 1e-5, "Primitive tolerance outside expected bounds.");
}
TopoDS_Shape cutBox() {
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
template<class Operation>
void expectInvalid(Operation operation) {
    try { operation(); }
    catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("Invalid repair arguments were accepted.");
}
} // namespace

int main() {
    try {
        const auto box = BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();
        const auto boxReport = nexsolid::diagnoseShape(box);
        checkCounts(boxReport, 6, 12, 8);
        checkRange(boxReport.vertexTolerances);
        checkRange(boxReport.edgeTolerances);
        checkRange(boxReport.faceTolerances);

        const auto cut = cutBox();
        const auto cutReport = nexsolid::diagnoseShape(cut);
        checkCounts(cutReport, 7, 15, 10);
        checkRange(cutReport.vertexTolerances);
        checkRange(cutReport.edgeTolerances);
        checkRange(cutReport.faceTolerances);

        for (const auto& original : {box, cut}) {
            const auto repaired = nexsolid::repairShape(original);
            require(!repaired.IsSame(original), "Repair must copy rather than mutate its input.");
            require(nexsolid::diagnoseShape(repaired).valid, "Repair returned invalid geometry.");
            near(volume(repaired), volume(original));
            const auto secondRepair = nexsolid::repairShape(repaired);
            near(volume(secondRepair), volume(original));
        }
        near(volume(box), 6000.0);
        checkCounts(nexsolid::diagnoseShape(box), 6, 12, 8);

        auto toleranceBox = BRepPrimAPI_MakeBox(1.0, 2.0, 3.0).Shape();
        BRep_Builder builder;
        TopExp_Explorer vertex(toleranceBox, TopAbs_VERTEX);
        TopExp_Explorer edge(toleranceBox, TopAbs_EDGE);
        TopExp_Explorer face(toleranceBox, TopAbs_FACE);
        builder.UpdateVertex(TopoDS::Vertex(vertex.Current()), 2e-4);
        builder.UpdateEdge(TopoDS::Edge(edge.Current()), 3e-4);
        builder.UpdateFace(TopoDS::Face(face.Current()), 4e-4);
        const auto ranges = nexsolid::diagnoseShape(toleranceBox);
        near(ranges.vertexTolerances.maximum, 2e-4, 1e-12);
        near(ranges.edgeTolerances.maximum, 3e-4, 1e-12);
        near(ranges.faceTolerances.maximum, 4e-4, 1e-12);
        require(ranges.vertexTolerances.minimum < ranges.vertexTolerances.maximum &&
                ranges.edgeTolerances.minimum < ranges.edgeTolerances.maximum &&
                ranges.faceTolerances.minimum < ranges.faceTolerances.maximum,
                "Minimum tolerance must include unmodified sub-shapes.");

        const auto empty = nexsolid::diagnoseShape(TopoDS_Shape());
        require(!empty.valid && empty.counts.vertices == 0 && empty.counts.edges == 0 &&
                empty.counts.faces == 0 && empty.counts.shells == 0 && empty.counts.solids == 0 &&
                empty.vertexTolerances.minimum == 0.0 && empty.faceTolerances.maximum == 0.0,
                "Null diagnostics should be safely empty.");
        expectInvalid([&] { nexsolid::repairShape(TopoDS_Shape()); });
        expectInvalid([&] { nexsolid::repairShape(box, 0.0); });
        expectInvalid([&] { nexsolid::repairShape(box, -1.0); });
        expectInvalid([&] { nexsolid::repairShape(box, 1e-3, 1e-4); });
        expectInvalid([&] { nexsolid::repairShape(box, std::numeric_limits<double>::quiet_NaN()); });
        expectInvalid([&] { nexsolid::repairShape(box, 1e-7, std::numeric_limits<double>::infinity()); });
        std::cout << "Topology, tolerance diagnostics and shape-repair checks passed.\n";
        return 0;
    } catch (const Standard_Failure& error) {
        std::cerr << "Unexpected OCCT failure: " << error.GetMessageString() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
    return 1;
}
