#include "nexsolid/Boolean.hpp"

#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Solid.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <cmath>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
using Operation = nexsolid::BooleanOperation;

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

double volume(const TopoDS_Shape& shape) {
    GProp_GProps properties;
    BRepGProp::VolumeProperties(shape, properties);
    return properties.Mass();
}

void near(double actual, double expected, const std::string& context, double tolerance = 1e-6) {
    require(std::isfinite(actual) && std::abs(actual - expected) <= tolerance,
            context + ": expected " + std::to_string(expected) + ", got " + std::to_string(actual));
}

std::string snapshot(const TopoDS_Shape& shape) {
    std::ostringstream stream;
    BRepTools::Write(shape, stream);
    return stream.str();
}

TopoDS_Shape box(double x, double y = 0.0, double z = 0.0, double side = 10.0) {
    return BRepPrimAPI_MakeBox(gp_Pnt(x, y, z), side, side, side).Shape();
}

TopoDS_Shape compound(const TopoDS_Shape& a = {}, const TopoDS_Shape& b = {}) {
    BRep_Builder builder;
    TopoDS_Compound result;
    builder.MakeCompound(result);
    if (!a.IsNull()) builder.Add(result, a);
    if (!b.IsNull()) builder.Add(result, b);
    return result;
}

void checkResult(const nexsolid::BooleanResult& result,
                 const TopoDS_Shape& a, const TopoDS_Shape& b,
                 double expected, const std::string& label) {
    require(!result.shape.IsNull(), label + ": result is null");
    require(BRepCheck_Analyzer(result.shape).IsValid(), label + ": invalid result");
    near(volume(result.shape), expected, label);
    TopTools_IndexedMapOfShape solids, original, output;
    TopExp::MapShapes(result.shape, TopAbs_SOLID, solids);
    require(result.solidCount == static_cast<std::size_t>(solids.Extent()),
            label + ": incorrect solid count");
    require(result.isEmpty == (std::abs(expected) <= 1e-9), label + ": incorrect empty-set status");
    TopExp::MapShapes(a, original);
    TopExp::MapShapes(b, original);
    TopExp::MapShapes(result.shape, output);
    for (int i = 1; i <= output.Extent(); ++i) {
        require(!original.Contains(output(i)), label + ": result aliases input topology");
    }
}

void matrix(const std::string& label, const TopoDS_Shape& a, const TopoDS_Shape& b,
            double intersection) {
    const double va = volume(a), vb = volume(b);
    const auto beforeA = snapshot(a), beforeB = snapshot(b);
    const auto fused = nexsolid::performBoolean(a, b, Operation::Fuse);
    const auto cut = nexsolid::performBoolean(a, b, Operation::Cut);
    const auto common = nexsolid::performBoolean(a, b, Operation::Common);
    checkResult(fused, a, b, va + vb - intersection, label + " fuse");
    checkResult(cut, a, b, va - intersection, label + " cut");
    checkResult(common, a, b, intersection, label + " common");
    near(volume(fused.shape) + volume(common.shape), va + vb, label + " union conservation");
    near(volume(cut.shape) + volume(common.shape), va, label + " cut conservation");
    // Commutative operations and directional subtraction must agree analytically.
    checkResult(nexsolid::performBoolean(b, a, Operation::Fuse), b, a,
                va + vb - intersection, label + " reverse fuse");
    checkResult(nexsolid::performBoolean(b, a, Operation::Common), b, a,
                intersection, label + " reverse common");
    checkResult(nexsolid::performBoolean(b, a, Operation::Cut), b, a,
                vb - intersection, label + " reverse cut");
    require(snapshot(a) == beforeA && snapshot(b) == beforeB, label + ": input modified");
    std::cout << "Passed: " << label << '\n';
}

template <typename Function>
void invalid(Function operation, const std::string& context) {
    try {
        operation();
    } catch (const std::invalid_argument& error) {
        require(std::string(error.what()).find(context) != std::string::npos,
                "Invalid-argument error lacks context: " + std::string(error.what()));
        return;
    }
    throw std::runtime_error("Invalid Boolean input was accepted: " + context);
}
} // namespace

int main() {
    try {
        const auto a = box(0.0);
        matrix("overlap", a, box(5.0), 500.0);
        matrix("disjoint", a, box(20.0), 0.0);
        matrix("face contact", a, box(10.0), 0.0);
        matrix("edge contact", a, box(10.0, 10.0), 0.0);
        matrix("vertex contact", a, box(10.0, 10.0, 10.0), 0.0);
        matrix("same topological object", a, a, 1000.0);
        matrix("independently coincident", a, box(0.0), 1000.0);
        matrix("contained", a, box(2.0, 2.0, 2.0, 3.0), 27.0);
        matrix("enclosing", a, box(-1.0, -1.0, -1.0, 12.0), 1000.0);
        matrix("thin intersection", a, box(10.0 - 1e-4), 0.01);
        matrix("nearly coincident", a, box(1e-4), 999.99);
        matrix("translated", box(10000.0, -20000.0, 30000.0),
               box(10005.0, -20000.0, 30000.0), 500.0);
        matrix("small-scale solids", box(0.0, 0.0, 0.0, 0.1),
               box(0.05, 0.0, 0.0, 0.1), 0.0005);
        const auto cylinder = BRepPrimAPI_MakeCylinder(
            gp_Ax2(gp_Pnt(5.0, 5.0, 0.0), gp_Dir(0.0, 0.0, 1.0)), 2.0, 10.0).Shape();
        matrix("box and through-cylinder", a, cylinder, std::acos(-1.0) * 40.0);

        // Empty results are successful values and remain usable in later operations.
        const auto empty = nexsolid::performBoolean(a, a, Operation::Cut).shape;
        matrix("empty operand identities", a, empty, 0.0);
        matrix("both operands empty", empty, compound(), 0.0);
        const auto distant = box(20.0);
        const auto assembly = nexsolid::performBoolean(a, distant, Operation::Fuse);
        require(assembly.solidCount == 2, "Disjoint fuse must preserve both solids");
        matrix("nested compound chaining", compound(compound(assembly.shape)), a, 1000.0);

        // A deliberate gap is retained by default and merged only with explicit fuzz.
        const auto gap = box(10.0001);
        const auto beforeA = snapshot(a), beforeGap = snapshot(gap);
        const auto separated = nexsolid::performBoolean(a, gap, Operation::Fuse);
        require(separated.solidCount == 2, "Default tolerance unexpectedly closed the gap");
        const auto fuzzy = nexsolid::performBoolean(a, gap, Operation::Fuse, {0.001});
        require(BRepCheck_Analyzer(fuzzy.shape).IsValid() && fuzzy.solidCount == 1,
                "Explicit fuzzy tolerance failed to resolve the near-coincident gap");
        // Fuzzy joining may alter geometry by the gap, so use its volume budget.
        near(volume(fuzzy.shape), 2000.0, "fuzzy gap volume budget", 0.02);
        require(snapshot(a) == beforeA && snapshot(gap) == beforeGap, "Fuzzy operation modified input");

        TopoDS_Solid emptySolid;
        BRep_Builder().MakeSolid(emptySolid);
        TopExp_Explorer faces(a, TopAbs_FACE);
        const auto face = faces.Current();
        const auto selfInterfering = compound(a, box(5.0));
        const auto mixedDimensions = compound(a, face);
        // BRep_Builder::Add freezes component topology. Construct all fixtures
        // before recording baselines so only API calls are measured below.
        const auto beforeFailures = snapshot(a);
        const auto badSnapshot = snapshot(selfInterfering);
        for (Operation op : {Operation::Fuse, Operation::Cut, Operation::Common}) {
            for (const auto& bad : {TopoDS_Shape(), face, mixedDimensions,
                                   TopoDS_Shape(emptySolid), a.Reversed(), selfInterfering}) {
                const auto beforeBad = bad.IsNull() ? std::string{} : snapshot(bad);
                invalid([&] { nexsolid::performBoolean(bad, a, op); }, "left operand");
                require(snapshot(a) == beforeFailures &&
                        (bad.IsNull() || snapshot(bad) == beforeBad),
                        "Rejecting a left operand modified input");
                invalid([&] { nexsolid::performBoolean(a, bad, op); }, "right operand");
                require(snapshot(a) == beforeFailures &&
                        (bad.IsNull() || snapshot(bad) == beforeBad),
                        "Rejecting a right operand modified input");
            }
            for (double tolerance : {-1.0, std::numeric_limits<double>::quiet_NaN(),
                                      std::numeric_limits<double>::infinity()}) {
                invalid([&] { nexsolid::performBoolean(a, a, op, {tolerance}); }, "fuzzy tolerance");
            }
        }
        invalid([&] { nexsolid::performBoolean(a, a, static_cast<Operation>(-1)); }, "operation");
        // A bad operand must not be silently accepted via the empty-set shortcut.
        invalid([&] { nexsolid::performBoolean(empty, selfInterfering, Operation::Common); },
                "right operand");
        require(snapshot(selfInterfering) == badSnapshot && snapshot(a) == beforeFailures,
                "Failure handling modified an input");
        matrix("success after rejected input", a, box(5.0), 500.0);
        std::cout << "Boolean operation matrix, empty sets, interference rejection and recovery passed.\n";
        return 0;
    } catch (const Standard_Failure& error) {
        std::cerr << "Untranslated OCCT exception: " << error.GetMessageString() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
    return 1;
}
