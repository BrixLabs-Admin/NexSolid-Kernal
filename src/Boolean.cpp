#include "nexsolid/Boolean.hpp"

#include <BOPAlgo_Operation.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <BRepAlgoAPI_Check.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRep_Builder.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace nexsolid {
namespace {

bool supportedTopology(const TopoDS_Shape& shape) {
    if (shape.ShapeType() == TopAbs_SOLID) return true;
    if (shape.ShapeType() != TopAbs_COMPOUND) return false;
    for (TopoDS_Iterator child(shape); child.More(); child.Next()) {
        if (!supportedTopology(child.Value())) return false;
    }
    return true;
}

std::string domainError(const TopoDS_Shape& shape) {
    if (shape.IsNull()) return "null shape";
    if (!supportedTopology(shape)) return "expected solids or nested compounds of solids";
    if (!BRepCheck_Analyzer(shape).IsValid()) return "invalid B-rep";
    TopTools_IndexedMapOfShape solids;
    TopExp::MapShapes(shape, TopAbs_SOLID, solids);
    for (int i = 1; i <= solids.Extent(); ++i) {
        GProp_GProps properties;
        BRepGProp::VolumeProperties(solids(i), properties);
        if (!std::isfinite(properties.Mass()) || properties.Mass() <= 0.0) {
            return "solid has non-positive or non-finite volume";
        }
    }
    return {};
}

std::size_t solidCount(const TopoDS_Shape& shape) {
    TopTools_IndexedMapOfShape solids;
    TopExp::MapShapes(shape, TopAbs_SOLID, solids);
    return static_cast<std::size_t>(solids.Extent());
}

TopoDS_Shape emptySet() {
    TopoDS_Compound shape;
    BRep_Builder().MakeCompound(shape);
    return shape;
}

TopoDS_Shape detachedCopy(const TopoDS_Shape& shape, std::size_t count) {
    if (count == 0) return emptySet();
    BRepBuilderAPI_Copy copy(shape, true, false);
    if (!copy.IsDone() || copy.Shape().IsNull()) {
        throw std::runtime_error("copy did not complete");
    }
    return copy.Shape();
}

} // namespace

BooleanResult performBoolean(const TopoDS_Shape& left,
                             const TopoDS_Shape& right,
                             BooleanOperation operation,
                             const BooleanOptions& options) {
    const char* name;
    BOPAlgo_Operation occtOperation;
    switch (operation) {
    case BooleanOperation::Fuse: name = "fuse"; occtOperation = BOPAlgo_FUSE; break;
    case BooleanOperation::Cut: name = "cut"; occtOperation = BOPAlgo_CUT; break;
    case BooleanOperation::Common: name = "common"; occtOperation = BOPAlgo_COMMON; break;
    default: throw std::invalid_argument("Unknown Boolean operation.");
    }
    const std::string prefix = std::string("Boolean ") + name;
    if (!std::isfinite(options.fuzzyTolerance) || options.fuzzyTolerance < 0.0) {
        throw std::invalid_argument(prefix + ": fuzzy tolerance must be finite and nonnegative.");
    }

    const char* stage = "input validation";
    try {
        const auto leftError = domainError(left), rightError = domainError(right);
        if (!leftError.empty()) throw std::invalid_argument(prefix + ": left operand: " + leftError);
        if (!rightError.empty()) throw std::invalid_argument(prefix + ": right operand: " + rightError);
        const auto leftCount = solidCount(left), rightCount = solidCount(right);
        stage = "copying operands";
        const auto leftCopy = detachedCopy(left, leftCount);
        const auto rightCopy = detachedCopy(right, rightCount);

        // Perform interference checks on copies, before accepting empty-set
        // shortcuts too. Small edges alone are not rejected as invalid.
        stage = "operand interference checks";
        if (leftCount != 0 && !BRepAlgoAPI_Check(leftCopy, false, true).IsValid()) {
            throw std::invalid_argument(prefix + ": left operand fails Boolean validity/self-interference checks.");
        }
        if (rightCount != 0 && !BRepAlgoAPI_Check(rightCopy, false, true).IsValid()) {
            throw std::invalid_argument(prefix + ": right operand fails Boolean validity/self-interference checks.");
        }

        BooleanResult result;
        if (leftCount == 0 || rightCount == 0) {
            if (operation == BooleanOperation::Fuse) {
                result.shape = leftCount == 0 ? rightCopy : leftCopy;
            } else if (operation == BooleanOperation::Cut) {
                result.shape = leftCopy;
            } else {
                result.shape = emptySet();
            }
        } else {
            stage = "building result";
            TopTools_ListOfShape arguments, tools;
            arguments.Append(leftCopy);
            tools.Append(rightCopy);
            BRepAlgoAPI_BooleanOperation builder;
            builder.SetArguments(arguments);
            builder.SetTools(tools);
            builder.SetOperation(occtOperation);
            builder.SetNonDestructive(true);
            builder.SetCheckInverted(true);
            builder.SetRunParallel(false);
            builder.SetFuzzyValue(options.fuzzyTolerance);
            builder.Build();
            if (!builder.IsDone() || builder.HasErrors()) {
                std::ostringstream errors;
                builder.DumpErrors(errors);
                throw std::runtime_error(errors.str().empty()
                    ? "OCCT did not complete the operation" : errors.str());
            }
            result.shape = builder.Shape();
            if (builder.HasWarnings()) {
                std::ostringstream warnings;
                builder.DumpWarnings(warnings);
                result.warnings = warnings.str();
            }
        }
        stage = "result validation";
        const auto error = domainError(result.shape);
        if (!error.empty()) throw std::runtime_error(error);
        result.solidCount = solidCount(result.shape);
        result.isEmpty = result.solidCount == 0;
        return result;
    } catch (const Standard_Failure& error) {
        const char* message = error.GetMessageString();
        throw std::runtime_error(prefix + " failed during " + stage + ": " +
                                 (message ? message : "OCCT exception"));
    } catch (const std::runtime_error& error) {
        throw std::runtime_error(prefix + " failed during " + stage + ": " + error.what());
    }
}

} // namespace nexsolid
