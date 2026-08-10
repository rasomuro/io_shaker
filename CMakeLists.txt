cmake_minimum_required(VERSION 3.15)

project(IOShaker LANGUAGES CXX)

add_library(IOShaker STATIC
    libRSO/IShaker.cpp
    libRSO/IShaker.h
    libRSO/RASH.cpp
    libRSO/RASH.h
    libRSO/RASHSolver.cpp
    libRSO/RASHSolver.h
    libRSO/RASH_bounds.h
    libRSO/Solver.cpp
    libRSO/Solver.h
    libRSO/function.cpp
    libRSO/function.h
    libRSO/shaker.cpp
    libRSO/shaker.h
    c_io_shaker.h
    c_io_shaker.cpp
)

add_library(IOShaker::IOShaker ALIAS IOShaker)

target_compile_features(IOShaker PUBLIC cxx_std_11)

target_include_directories(IOShaker
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libRSO>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(IOShaker PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)
