cmake_minimum_required(VERSION 3.15)

project(RSO LANGUAGES CXX)

add_library(RSO STATIC
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
    lion.h
    lion.cpp
)

add_library(RSO::RSO ALIAS RSO)

target_compile_features(RSO PUBLIC cxx_std_11)

target_include_directories(RSO
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libRSO>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(RSO PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)
