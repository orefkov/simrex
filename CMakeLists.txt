cmake_minimum_required(VERSION 3.20)
include(cmake/prelude.cmake)
include(CMakeDependentOption)
include(FetchContent)

project(
    simrex
    VERSION 1.0.0
    DESCRIPTION "Wrapper for simstr strings for Oniguruma regexp"
    HOMEPAGE_URL "https://github.com/orefkov/simrex"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(MSVC_COMPILER ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /Zc:strictStrings")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    set(CLANG_COMPILER ON)
endif()

option(SIMREX_BUILD_TESTS "Построить тесты" ON)

add_library(simrex_simrex
    src/onig.cpp
)
add_library(simrex::simrex ALIAS simrex_simrex)

target_include_directories(
    simrex_simrex ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(simrex_simrex PUBLIC cxx_std_20)

set_target_properties(
    simrex_simrex PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME simrex
    OUTPUT_NAME simrex
)

function(add_simstr)
    set(SIMSTR_BUILD_TESTS OFF)
    set(SIMSTR_BENCHMARKS OFF)
    if(SIMREX_IN_SHARED)
        set(SIMSTR_IN_SHARED On)
    endif()
    FetchContent_Declare(
        simstr
        GIT_REPOSITORY https://github.com/orefkov/simstr.git
        GIT_SHALLOW TRUE
        GIT_TAG 4ddb1ae
        FIND_PACKAGE_ARGS NAMES simstr
    )
    FetchContent_MakeAvailable(simstr)
endfunction()

function(add_oniguruma)
    set (BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set (INSTALL_DOCUMENTATION OFF CACHE BOOL "" FORCE)
    set (BUILD_TEST OFF CACHE BOOL "" FORCE)
    #set (MSVC_STATIC_RUNTIME ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        oniguruma
        GIT_REPOSITORY https://github.com/kkos/oniguruma.git
        GIT_SHALLOW TRUE
        GIT_TAG v6.9.10
        FIND_PACKAGE_ARGS NAMES oniguruma 
    )
    FetchContent_MakeAvailable(oniguruma)
    if (NOT ${oniguruma_FOUND})
        add_library(oniguruma::onig ALIAS onig)
    endif()
endfunction(add_oniguruma)

add_simstr()
add_oniguruma()

target_link_libraries(simrex_simrex PUBLIC oniguruma::onig simstr::simstr)

if(BUILD_SHARED_LIBS)
    # Всем объявляем, что мы будем в shared библиотеке
    add_compile_definitions(SIMREX_IN_SHARED)
    # Себе объявим, что мы должны экспортировать функции
    target_compile_definitions(simrex_simrex PRIVATE SIMREX_EXPORT)
endif(SIMREX_IN_SHARED)

if(SIMREX_BUILD_TESTS)
    enable_testing()
    set(BUILD_TESTS ON)
    # Load and build GTest
    FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        FIND_PACKAGE_ARGS NAMES GTest
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt FALSE CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT simrex_DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(
        AUTHOR_WARNING
        "Developer mode is intended for developers of simrex"
    )
endif()

include(cmake/dev-mode.cmake)
