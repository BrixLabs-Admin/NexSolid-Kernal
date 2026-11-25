#pragma once

#include <TopoDS_Shape.hxx>
#include <filesystem>

namespace nexsolid {

// Transfer the shape using STEPControl_AsIs. Null/invalid shapes are rejected.
// Filesystem/translation failures are reported as std::runtime_error.
void writeStep(const TopoDS_Shape& shape, const std::filesystem::path& filename);

// Read all transferable roots; reject missing, corrupt, empty or invalid models.
TopoDS_Shape readStep(const std::filesystem::path& filename);

} // namespace nexsolid
