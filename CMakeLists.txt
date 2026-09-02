cmake_minimum_required(VERSION 3.16)

project(
    MaintenanceHierarchicalDecompositions
    VERSION 0.1.0
    LANGUAGES CXX
)

option(
    MHD_NATIVE_OPTIMIZATION
    "Enable architecture-specific compiler optimizations"
    OFF
)
option(
    MHD_ENABLE_IPO
    "Enable interprocedural optimization (LTO)"
    OFF
)

find_package(OpenMP REQUIRED COMPONENTS CXX)

add_library(mhd_core STATIC
    src/analysis/AY.cpp
    models/core/CoreTree.cpp
    src/utils/GraphUtils.cpp
    src/graph/ParaGraph.cpp
    src/tree/TreeBuildIndex.cpp
    src/profile/AlgoProfile.cpp
    models/trine/Trine.cpp
    src/tree/TrineTree.cpp
    models/trine/detail/order.cpp
    models/trine/detail/thebin.cpp
    models/trine/detail/theheap.cpp

    include/analysis/AY.hpp
    models/core/CoreTree.hpp
    graph/Graph.hpp
    graph/ParaGraph.hpp
    include/tree/TreeBuildIndex.hpp
    utils/GraphUtils.h
    include/profile/AlgoProfile.hpp
    models/trine/Trine.hpp
    include/tree/TrineTree.hpp
    models/trine/detail/defs.h
    models/trine/detail/order.h
    models/trine/detail/thebin.h
    models/trine/detail/theheap.h
)

target_compile_features(mhd_core PUBLIC cxx_std_14)

target_include_directories(mhd_core
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include/analysis"
        "${PROJECT_SOURCE_DIR}/models/core"
        "${PROJECT_SOURCE_DIR}/graph"
        "${PROJECT_SOURCE_DIR}/utils"
        "${PROJECT_SOURCE_DIR}/include/profile"
        "${PROJECT_SOURCE_DIR}/models/trine"
        "${PROJECT_SOURCE_DIR}/models/trine/detail"
        "${PROJECT_SOURCE_DIR}/include/tree"
)

target_link_libraries(mhd_core
    PUBLIC
        OpenMP::OpenMP_CXX
)

if(MHD_NATIVE_OPTIMIZATION)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mhd_core PUBLIC -march=native)
    else()
        message(WARNING
            "MHD_NATIVE_OPTIMIZATION is not supported by "
            "${CMAKE_CXX_COMPILER_ID}"
        )
    endif()
endif()

if(MHD_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT MHD_IPO_SUPPORTED
                        OUTPUT MHD_IPO_ERROR)
    if(MHD_IPO_SUPPORTED)
        set_property(TARGET mhd_core
                     PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(FATAL_ERROR "IPO/LTO is not supported: ${MHD_IPO_ERROR}")
    endif()
endif()

add_executable(mhd
    src/app/main.cpp
)

target_link_libraries(mhd
    PRIVATE
        mhd_core
)

if(MHD_ENABLE_IPO AND MHD_IPO_SUPPORTED)
    set_property(TARGET mhd
                 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
