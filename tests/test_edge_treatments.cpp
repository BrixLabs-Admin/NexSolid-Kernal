#include "nexsolid/EdgeTreatments.hpp"

#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>

#include <cmath>
#include <exception>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void near(double actual, double expected) {
    require(std::isfinite(actual) && std::abs(actual - expected) <= 1e-6,
            "Edge-treatment volume differs from its analytical value.");
}

double volume(const TopoDS_Shape& shape) {
    GProp_GProps properties;
    BRepGProp::VolumeProperties(shape, properties);
    return properties.Mass();
}

std::vector<TopoDS_Edge> allEdges(const TopoDS_Shape& shape) {
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(shape, TopAbs_EDGE, map);
    std::vector<TopoDS_Edge> edges;
    for (int i = 1; i <= map.Extent(); ++i) edges.push_back(TopoDS::Edge(map(i)));
    return edges;
}

std::string snapshot(const TopoDS_Shape& shape) {
    std::ostringstream stream;
    BRepTools::Write(shape, stream);
    return stream.str();
}

void validReduction(const TopoDS_Shape& result) {
    require(!result.IsNull() && BRepCheck_Analyzer(result).IsValid(),
            "Edge treatment returned an invalid shape.");
    require(volume(result) > 0.0 && volume(result) < 1000.0,
            "A convex box treatment must reduce its positive volume.");
    TopTools_IndexedMapOfShape solids;
    TopExp::MapShapes(result, TopAbs_SOLID, solids);
    require(solids.Extent() == 1, "Expected one resulting solid.");
}

template <typename Exception, typename Function>
void expectError(Function operation, const char* name) {
    try {
        operation();
    } catch (const Exception& error) {
        require(std::string(error.what()).find(name) != std::string::npos,
                "Error does not identify the failing operation.");
        return;
    }
    throw std::runtime_error("Invalid or impossible edge treatment unexpectedly succeeded.");
}

} // namespace

int main() {
    try {
        const TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
        const auto edges = allEdges(box);
        require(edges.size() == 12, "Expected twelve unique box edges.");
        const std::vector<TopoDS_Edge> oneEdge{edges.front()};
        const std::string original = snapshot(box);
        const double pi = std::acos(-1.0);
        TopTools_IndexedMapOfShape originalTopology;
        TopExp::MapShapes(box, originalTopology);
        const auto detached = [&](const TopoDS_Shape& result) {
            TopTools_IndexedMapOfShape resultTopology;
            TopExp::MapShapes(result, resultTopology);
            for (int i = 1; i <= resultTopology.Extent(); ++i) {
                require(!originalTopology.Contains(resultTopology(i)),
                        "Result shares mutable topology with the input.");
            }
        };

        // One straight convex edge: removed cross section is a square minus
        // a quarter circle for a fillet, or a right triangle for a chamfer.
        for (double size : {0.05, 1.0}) {
            const auto fillet = nexsolid::filletEdges(box, oneEdge, size);
            validReduction(fillet);
            detached(fillet);
            near(volume(fillet), 1000.0 - (1.0 - pi / 4.0) * size * size * 10.0);
            const auto chamfer = nexsolid::chamferEdges(box, oneEdge, size);
            validReduction(chamfer);
            detached(chamfer);
            near(volume(chamfer), 1000.0 - 0.5 * size * size * 10.0);
            require(snapshot(box) == original, "Successful operation changed the input.");
        }

        validReduction(nexsolid::filletEdges(box, edges, 1.0));
        validReduction(nexsolid::chamferEdges(box, edges, 1.0));
        require(snapshot(box) == original, "Multi-edge operation changed the input.");

        // No artificial upper-size guard: these reach OCCT's builder, exceed
        // half the box width, and must report failure without leaking an
        // OCCT exception or altering the original geometry.
        expectError<std::runtime_error>(
            [&] { nexsolid::filletEdges(box, edges, 6.0); }, "fillet");
        require(snapshot(box) == original, "Failed fillet changed the input.");
        expectError<std::runtime_error>(
            [&] { nexsolid::chamferEdges(box, edges, 6.0); }, "chamfer");
        require(snapshot(box) == original, "Failed chamfer changed the input.");

        // The same input must still support successful operations after failure.
        validReduction(nexsolid::filletEdges(box, oneEdge, 1.0));
        validReduction(nexsolid::chamferEdges(box, oneEdge, 1.0));
        require(snapshot(box) == original, "Recovery changed the input.");
        near(volume(box), 1000.0);

        const auto foreign = allEdges(BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape());
        TopoDS_Solid emptySolid;
        BRep_Builder().MakeSolid(emptySolid);
        for (bool fillet : {true, false}) {
            const char* name = fillet ? "fillet" : "chamfer";
            const auto apply = [&](const TopoDS_Shape& shape,
                                   const std::vector<TopoDS_Edge>& selection, double size) {
                return fillet ? nexsolid::filletEdges(shape, selection, size)
                              : nexsolid::chamferEdges(shape, selection, size);
            };
            for (double size : {0.0, -1.0, std::numeric_limits<double>::quiet_NaN(),
                                std::numeric_limits<double>::infinity()}) {
                expectError<std::invalid_argument>([&] { apply(box, oneEdge, size); }, name);
            }
            expectError<std::invalid_argument>([&] { apply(TopoDS_Shape(), oneEdge, 1.0); }, name);
            expectError<std::invalid_argument>([&] { apply(emptySolid, oneEdge, 1.0); }, name);
            expectError<std::invalid_argument>([&] { apply(edges.front(), oneEdge, 1.0); }, name);
            expectError<std::invalid_argument>([&] { apply(box.Reversed(), oneEdge, 1.0); }, name);
            expectError<std::invalid_argument>([&] { apply(box, {}, 1.0); }, name);
            expectError<std::invalid_argument>([&] { apply(box, {TopoDS_Edge()}, 1.0); }, name);
            expectError<std::invalid_argument>([&] { apply(box, {foreign.front()}, 1.0); }, name);
            expectError<std::invalid_argument>(
                [&] { apply(box, {edges.front(), edges.front()}, 1.0); }, name);
        }
        require(snapshot(box) == original, "Argument validation changed the input.");
        std::cout << "Fillet/chamfer geometry, input preservation and failure recovery passed.\n";
        return 0;
    } catch (const Standard_Failure& error) {
        std::cerr << "Unexpected OCCT exception: " << error.GetMessageString() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
    return 1;
}
