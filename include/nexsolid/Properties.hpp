#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <array>
#include <vector>

namespace nexsolid {

struct MassProperties {
    double volume = 0.0;
    double surfaceArea = 0.0;
    double edgeLength = 0.0; // Shared topological edges counted once.
    gp_Pnt centerOfMass;
    // Unit-density, centroidal principal moments, in ascending order.
    std::array<double, 3> principalMoments{};
};

// Requires valid geometry containing closed, positive-volume solids.
MassProperties massProperties(const TopoDS_Shape& shape);

struct IndexedSubShape {
    TopoDS_Shape shape;
    gp_Pnt centroid;
    double measure = 0.0; // Volume, area, length, or zero for vertices.
    std::array<double, 6> bounds{};
    // Quantized centroid, measure and bounds; no pointer/encounter-order keys.
    std::array<double, 10> geometricKey{};
};

struct TopologyIndex {
    std::vector<IndexedSubShape> solids, shells, faces, edges, vertices;
    // Equal keys are explicitly ambiguous: their object identities cannot be
    // ordered using these geometric invariants alone.
    bool hasAmbiguousKeys = false;
};

// Deterministic ordering for distinct geometric keys. keyTolerance is an
// absolute quantization grid applied componentwise to the geometric values.
// This is not persistent topological naming across modeling operations.
TopologyIndex indexTopology(const TopoDS_Shape& shape, double keyTolerance = 1e-9);

} // namespace nexsolid
