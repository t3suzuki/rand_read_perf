cmake_minimum_required (VERSION 3.10)
project(mybench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)

if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cachelib-sosp23/cachelib/cmake/")
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cachelib-sosp23/opt/cachelib/")

message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# add_subdirectory(../cachelib cachelib)

find_package(cachelib CONFIG REQUIRED)
find_package(fmt)

add_definitions("-DENABLE_S3FIFO=1")

if (DEFINED N_TH)
  add_definitions("-DN_TH=${N_TH}")
endif()

if (DEFINED URAND)
  add_definitions("-DURAND=${URAND}")
endif()

if (DEFINED THETA)
  add_definitions("-DTHETA=${THETA}")
endif()

add_executable(my main.cc nvme.cc)
target_link_libraries(my cachelib fmt::fmt atomic) 
