#################
# Configuration #
#################

# CMake

cmake_minimum_required(VERSION 3.12)

# Project

project(uHash
        VERSION 0.1.0
        DESCRIPTION "A type safe, generic C hash table."
        HOMEPAGE_URL "https://github.com/IvanoBilenchi/uhash"
        LANGUAGES C)

# Files and directories

set(UH_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(UH_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(UH_INCLUDE_DIR "${UH_PROJECT_DIR}/include")
set(UH_DOCS_DIR "${UH_PROJECT_DIR}/docs")
set(UH_TEST_DIR "${UH_PROJECT_DIR}/test")
set(UH_HEADER "${UH_INCLUDE_DIR}/uhash.h")

###############
### Targets ###
###############

# Interface library

add_library(uhash INTERFACE)
target_sources(uhash INTERFACE "${UH_HEADER}")
target_include_directories(uhash INTERFACE "${UH_INCLUDE_DIR}")

# Subprojects

add_subdirectory("${UH_TEST_DIR}")
add_subdirectory("${UH_DOCS_DIR}")
