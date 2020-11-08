#################
# Configuration #
#################

# CMake

cmake_minimum_required(VERSION 3.13)

# Project

project(uHash
        VERSION 0.1.0
        DESCRIPTION "A type safe, generic C hash table."
        HOMEPAGE_URL "https://github.com/IvanoBilenchi/uhash"
        LANGUAGES C)

###############
### Targets ###
###############

# Interface library

add_library(uhash INTERFACE)
target_sources(uhash INTERFACE "include/uhash.h")
target_include_directories(uhash INTERFACE "include")

# Subprojects

add_subdirectory("test")
add_subdirectory("docs")
