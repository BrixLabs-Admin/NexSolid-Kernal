cmake_minimum_required(VERSION 3.20)

project(NexSolidKernel
    VERSION 1.1.0
    DESCRIPTION "An early-stage CAD geometry kernel starter built on OCCT"
    LANGUAGES CXX
)

include(CTest)

# Optional, isolated OCCT installation built by a contributor. When set,
# NEVER fall back to system OCCT: that could falsely validate a kernel patch.
set(NEXSOLID_OCCT_ROOT "" CACHE PATH
    "Path to an installed OCCT prefix to test instead of system OCCT")

set(NEXSOLID_OCCT_TOOLKITS
    TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep
    TKGeomAlgo TKTopAlgo TKPrim
)

if(NEXSOLID_OCCT_ROOT)
    get_filename_component(NEXSOLID_OCCT_ROOT "${NEXSOLID_OCCT_ROOT}" ABSOLUTE)
    find_path(NEXSOLID_OCCT_INCLUDE_DIR
        NAMES Standard_Version.hxx
        HINTS
            "${NEXSOLID_OCCT_ROOT}"
            "${NEXSOLID_OCCT_ROOT}/include"
            "${NEXSOLID_OCCT_ROOT}/inc"
        PATH_SUFFIXES opencascade include/opencascade inc
        NO_DEFAULT_PATH
    )
else()
    # Some distro development packages have no OpenCASCADEConfig.cmake.
    find_package(OpenCASCADE QUIET CONFIG)
    find_path(NEXSOLID_OCCT_INCLUDE_DIR
        NAMES Standard_Version.hxx
        HINTS ${OpenCASCADE_INCLUDE_DIR}
        PATH_SUFFIXES opencascade include/opencascade inc
    )
endif()

if(NOT NEXSOLID_OCCT_INCLUDE_DIR)
    message(FATAL_ERROR
        "OCCT headers not found. Install OCCT development packages or set "
        "NEXSOLID_OCCT_ROOT to an installed OCCT prefix.")
endif()

set(NEXSOLID_OCCT_LIBRARIES)
foreach(toolkit IN LISTS NEXSOLID_OCCT_TOOLKITS)
    if(NEXSOLID_OCCT_ROOT)
        find_library(NEXSOLID_OCCT_${toolkit}_LIBRARY
            NAMES ${toolkit}
            HINTS
                "${NEXSOLID_OCCT_ROOT}/lib"
                "${NEXSOLID_OCCT_ROOT}/lib64"
                "${NEXSOLID_OCCT_ROOT}/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
            NO_DEFAULT_PATH
        )
    else()
        find_library(NEXSOLID_OCCT_${toolkit}_LIBRARY
            NAMES ${toolkit}
            HINTS ${OpenCASCADE_LIBRARY_DIR}
        )
    endif()
    if(NOT NEXSOLID_OCCT_${toolkit}_LIBRARY)
        message(FATAL_ERROR
            "OCCT toolkit ${toolkit} not found. Install Foundation, "
            "Modeling Data and Modeling Algorithms development packages, "
            "or provide a complete installation with NEXSOLID_OCCT_ROOT.")
    endif()
    list(APPEND NEXSOLID_OCCT_LIBRARIES
        "${NEXSOLID_OCCT_${toolkit}_LIBRARY}")
endforeach()

message(STATUS "NexSolid OCCT headers: ${NEXSOLID_OCCT_INCLUDE_DIR}")
message(STATUS "NexSolid OCCT toolkits: ${NEXSOLID_OCCT_LIBRARIES}")
if(NEXSOLID_OCCT_ROOT)
    message(STATUS "NexSolid isolated OCCT prefix: ${NEXSOLID_OCCT_ROOT}")
endif()

add_library(nexsolid_kernel
    src/Kernel.cpp
    src/Properties.cpp
)
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

    add_executable(nexsolid_properties_tests tests/test_properties.cpp)
    target_link_libraries(nexsolid_properties_tests PRIVATE NexSolid::Kernel)
    add_test(NAME properties COMMAND nexsolid_properties_tests)
    set_tests_properties(properties PROPERTIES TIMEOUT 120)
endif()
