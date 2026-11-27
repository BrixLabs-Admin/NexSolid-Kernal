#pragma once

#include <TopoDS_Shape.hxx>
#include <cstddef>
#include <string>

namespace nexsolid {

enum class BooleanOperation { Fuse, Cut, Common };

struct BooleanOptions {
    // Additional OCCT fuzzy tolerance in model units. Must be finite and >= 0.
    // Zero retains OCCT's baseline tolerances; no automatic tolerance retries.
    double fuzzyTolerance = 0.0;
};

struct BooleanResult {
    TopoDS_Shape shape; // Non-null, validated result, including empty compounds.
    bool isEmpty = false;
    std::size_t solidCount = 0;
    std::string warnings; // OCCT warnings are preserved even on successful builds.
};

// Operands are valid, positive-volume solids or nested compounds of such
// solids (including empty compounds). Self-interfering operands are rejected.
// Faces, shells, wires, compsolids, null shapes and inverted solids are rejected.
// Cut computes left minus right. Empty sets obey the usual set identities.
//
// Geometry/topology copies isolate both inputs and the returned result.
// Throws std::invalid_argument for bad input/options; std::runtime_error for
// OCCT failures or invalid results, with operation/stage context.
// Runs serially. Success does not promise persistent subshape identities,
// exact arithmetic, or robustness for every pathological B-rep.
BooleanResult performBoolean(const TopoDS_Shape& left,
                             const TopoDS_Shape& right,
                             BooleanOperation operation,
                             const BooleanOptions& options = {});

} // namespace nexsolid
