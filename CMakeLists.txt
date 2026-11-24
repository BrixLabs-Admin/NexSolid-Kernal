cmake_minimum_required(VERSION 3.20)

project(NexSolidKernel
    VERSION 0.1.0
    DESCRIPTION "An early-stage CAD geometry kernel starter built on OCCT"
    LANGUAGES CXX
)

include(CTest)

# Source-built OCCT usually provides OpenCASCADEConfig.cmake. Distribution
# packages (including Ubuntu 22.04) may only provide headers and libraries.
# Support both layouts so contributors can build without repackaging OCCT.
find_package(OpenCASCADE QUIET CONFIG)

find_path(NEXSOLID_OCCT_INCLUDE_DIR
    NAMES Standard_Version.hxx
    HINTS ${OpenCASCADE_INCLUDE_DIR}
    PATH_SUFFIXES opencascade include/opencascade inc
)
if(NOT NEXSOLID_OCCT_INCLUDE_DIR)
    message(FATAL_ERROR
        "OCCT headers not found. Install OCCT development packages or set "
        "CMAKE_PREFIX_PATH to your OCCT installation.")
endif()

set(NEXSOLID_OCCT_TOOLKITS
    TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep
    TKGeomAlgo TKTopAlgo TKPrim
)
set(NEXSOLID_OCCT_LIBRARIES)
foreach(toolkit IN LISTS NEXSOLID_OCCT_TOOLKITS)
    find_library(NEXSOLID_OCCT_${toolkit}_LIBRARY
        NAMES ${toolkit}
        HINTS ${OpenCASCADE_LIBRARY_DIR}
    )
    if(NOT NEXSOLID_OCCT_${toolkit}_LIBRARY)
        message(FATAL_ERROR
            "OCCT library ${toolkit} not found. Install Foundation, "
            "Modeling Data and Modeling Algorithms development packages, "
            "or set CMAKE_PREFIX_PATH to the matching OCCT installation.")
    endif()
    list(APPEND NEXSOLID_OCCT_LIBRARIES
        "${NEXSOLID_OCCT_${toolkit}_LIBRARY}")
endforeach()

message(STATUS "NexSolid using OCCT headers: ${NEXSOLID_OCCT_INCLUDE_DIR}")
message(STATUS "NexSolid using OCCT toolkits: ${NEXSOLID_OCCT_LIBRARIES}")

add_library(nexsolid_kernel src/Kernel.cpp)
add_library(NexSolid::Kernel ALIAS nexsolid_kernel)

target_compile_features(nexsolid_kernel PUBLIC cxx_std_17)
target_include_directories(nexsolid_kernel
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${NEXSOLID_OCCT_INCLUDE_DIR}
)
target_link_libraries(nexsolid_kernel PUBLIC ${NEXSOLID_OCCT_LIBRARIES})

add_executable(nexsolid_demo app/main.cpp)
target_link_libraries(nexsolid_demo PRIVATE NexSolid::Kernel)

if(BUILD_TESTING)
    add_executable(nexsolid_kernel_tests tests/test_kernel.cpp)
    target_link_libraries(nexsolid_kernel_tests PRIVATE NexSolid::Kernel)
    add_test(NAME kernel_smoke COMMAND nexsolid_kernel_tests)
endif()
