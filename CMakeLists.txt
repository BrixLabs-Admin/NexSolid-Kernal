cmake_minimum_required(VERSION 3.20)

project(NexSolidKernel
    VERSION 0.1.0
    DESCRIPTION "An early-stage CAD geometry kernel starter built on OCCT"
    LANGUAGES CXX
)

include(CTest)
find_package(OpenCASCADE REQUIRED)

add_library(nexsolid_kernel
    src/Kernel.cpp
)
add_library(NexSolid::Kernel ALIAS nexsolid_kernel)

target_compile_features(nexsolid_kernel PUBLIC cxx_std_17)
target_include_directories(nexsolid_kernel
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${OpenCASCADE_INCLUDE_DIR}
)
target_link_libraries(nexsolid_kernel
    PUBLIC
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKBRep
        TKGeomAlgo
        TKTopAlgo
        TKPrim
)

add_executable(nexsolid_demo app/main.cpp)
target_link_libraries(nexsolid_demo PRIVATE NexSolid::Kernel)

if(BUILD_TESTING)
    add_executable(nexsolid_kernel_tests tests/test_kernel.cpp)
    target_link_libraries(nexsolid_kernel_tests PRIVATE NexSolid::Kernel)
    add_test(NAME kernel_smoke COMMAND nexsolid_kernel_tests)
endif()
