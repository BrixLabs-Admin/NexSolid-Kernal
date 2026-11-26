#include "nexsolid/StepIO.hpp"

#include <BRepCheck_Analyzer.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Writer.hxx>
#include <Standard_Failure.hxx>

#include <mutex>
#include <stdexcept>
#include <string>
#include <system_error>

namespace nexsolid {
namespace {
std::mutex stepMutex;

std::runtime_error failure(const char* operation, const std::filesystem::path& file) {
    return std::runtime_error(std::string("STEP ") + operation + " failed: " + file.string());
}
} // namespace

void writeStep(const TopoDS_Shape& shape, const std::filesystem::path& filename) {
    if (shape.IsNull() || filename.empty()) {
        throw std::invalid_argument("STEP export requires a non-null shape and a filename.");
    }
    std::lock_guard<std::mutex> lock(stepMutex);
    try {
        if (!BRepCheck_Analyzer(shape).IsValid()) {
            throw std::invalid_argument("STEP export requires a valid shape.");
        }
        STEPControl_Writer writer;
        if (writer.Transfer(shape, STEPControl_AsIs) != IFSelect_RetDone) {
            throw failure("transfer for export", filename);
        }
        if (writer.Write(filename.string().c_str()) != IFSelect_RetDone) {
            throw failure("write", filename);
        }
    } catch (const Standard_Failure& error) {
        throw std::runtime_error(std::string("STEP export failed: ") +
                                 filename.string() + ": " + error.GetMessageString());
    }
}

TopoDS_Shape readStep(const std::filesystem::path& filename) {
    std::error_code error;
    if (filename.empty() || !std::filesystem::is_regular_file(filename, error)) {
        throw failure("read (file missing or not a regular file)", filename);
    }
    std::lock_guard<std::mutex> lock(stepMutex);
    try {
        STEPControl_Reader reader;
        if (reader.ReadFile(filename.string().c_str()) != IFSelect_RetDone) {
            throw failure("read (invalid STEP data)", filename);
        }
        const int roots = reader.NbRootsForTransfer();
        if (roots <= 0 || reader.TransferRoots() != roots) {
            throw failure("transfer of all roots", filename);
        }
        const TopoDS_Shape shape = reader.OneShape();
        if (shape.IsNull() || !BRepCheck_Analyzer(shape).IsValid()) {
            throw failure("read (empty or invalid result)", filename);
        }
        return shape;
    } catch (const Standard_Failure& exception) {
        throw std::runtime_error(std::string("STEP import failed: ") +
                                 filename.string() + ": " + exception.GetMessageString());
    }
}

} // namespace nexsolid
