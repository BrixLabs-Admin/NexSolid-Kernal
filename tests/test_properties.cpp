#include "nexsolid/Properties.hpp"

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRep_Builder.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Pnt.hxx>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void near(double actual, double expected) {
    require(std::isfinite(actual) && std::abs(actual - expected) <= 1e-6,
            "Analytical mass-property check failed.");
}
void sameRecords(const std::vector<nexsolid::IndexedSubShape>& a,
                 const std::vector<nexsolid::IndexedSubShape>& b) {
    require(a.size() == b.size(), "Topology count changed.");
    for (std::size_t i = 0; i < a.size(); ++i) {
        require(a[i].geometricKey == b[i].geometricKey, "Geometric ordering changed.");
        near(a[i].centroid.Distance(b[i].centroid), 0.0);
        near(a[i].measure, b[i].measure);
        for (std::size_t j = 0; j < a[i].bounds.size(); ++j) near(a[i].bounds[j], b[i].bounds[j]);
        if (i > 0) require(a[i - 1].geometricKey <= a[i].geometricKey, "Index is not sorted.");
    }
}
void sameIndex(const nexsolid::TopologyIndex& a, const nexsolid::TopologyIndex& b) {
    sameRecords(a.solids, b.solids);
    sameRecords(a.shells, b.shells);
    sameRecords(a.faces, b.faces);
    sameRecords(a.edges, b.edges);
    sameRecords(a.vertices, b.vertices);
}
TopoDS_Compound compound(const TopoDS_Shape& first, const TopoDS_Shape& second) {
    TopoDS_Compound result;
    BRep_Builder builder;
    builder.MakeCompound(result);
    builder.Add(result, first);
    builder.Add(result, second);
    return result;
}
template<class Function>
void expectInvalid(Function operation) {
    try { operation(); }
    catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("Invalid property request was accepted.");
}
} // namespace

int main() {
    try {
        const auto box = BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();
        const auto properties = nexsolid::massProperties(box);
        near(properties.volume, 6000.0);
        near(properties.surfaceArea, 2200.0);
        near(properties.edgeLength, 240.0);
        near(properties.centerOfMass.X(), 5.0);
        near(properties.centerOfMass.Y(), 10.0);
        near(properties.centerOfMass.Z(), 15.0);
        near(properties.principalMoments[0], 250000.0);
        near(properties.principalMoments[1], 500000.0);
        near(properties.principalMoments[2], 650000.0);

        const auto reference = nexsolid::indexTopology(box);
        require(reference.solids.size() == 1 && reference.shells.size() == 1 &&
                reference.faces.size() == 6 && reference.edges.size() == 12 &&
                reference.vertices.size() == 8, "Incorrect box index counts.");
        require(!reference.hasAmbiguousKeys, "Asymmetric box should have distinct keys.");
        for (int i = 0; i < 5; ++i) {
            sameIndex(reference, nexsolid::indexTopology(box));
            sameIndex(reference, nexsolid::indexTopology(
                BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape()));
        }

        const auto translated = BRepPrimAPI_MakeBox(gp_Pnt(2.0, -4.0, 6.0), 10.0, 20.0, 30.0).Shape();
        const auto moved = nexsolid::massProperties(translated);
        near(moved.centerOfMass.X(), 7.0);
        near(moved.centerOfMass.Y(), 6.0);
        near(moved.centerOfMass.Z(), 21.0);
        for (std::size_t i = 0; i < 3; ++i) near(moved.principalMoments[i], properties.principalMoments[i]);

        const auto other = BRepPrimAPI_MakeBox(gp_Pnt(40.0, 0.0, 0.0), 10.0, 20.0, 30.0).Shape();
        const auto forward = nexsolid::indexTopology(compound(box, other));
        const auto backward = nexsolid::indexTopology(compound(other, box));
        require(!forward.hasAmbiguousKeys && !backward.hasAmbiguousKeys,
                "Disjoint boxes should have distinct keys.");
        sameIndex(forward, backward);
        near(nexsolid::massProperties(compound(box, other)).volume, 12000.0);

        const auto duplicate = BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();
        require(nexsolid::indexTopology(compound(box, duplicate)).hasAmbiguousKeys,
                "Geometrically indistinguishable topology must be flagged.");
        const auto vertex = nexsolid::indexTopology(BRepBuilderAPI_MakeVertex(gp_Pnt(1.0, 2.0, 3.0)).Shape());
        require(vertex.vertices.size() == 1, "A root vertex must be indexed.");
        near(vertex.vertices[0].centroid.X(), 1.0);

        expectInvalid([&] { nexsolid::massProperties(TopoDS_Shape()); });
        expectInvalid([&] { nexsolid::indexTopology(TopoDS_Shape()); });
        expectInvalid([&] { nexsolid::indexTopology(box, 0.0); });
        expectInvalid([&] { nexsolid::indexTopology(box, std::numeric_limits<double>::quiet_NaN()); });
        expectInvalid([&] { nexsolid::indexTopology(box, std::numeric_limits<double>::infinity()); });
        TopExp_Explorer face(box, TopAbs_FACE);
        expectInvalid([&] { nexsolid::massProperties(face.Current()); });
        std::cout << "Deterministic topology and analytical mass-property checks passed.\n";
        return 0;
    } catch (const Standard_Failure& error) {
        std::cerr << "Unexpected OCCT failure: " << error.GetMessageString() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
    return 1;
}
