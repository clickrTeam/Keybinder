cmake_minimum_required(VERSION 3.19)

set(COMMON_SOURCES
    src/main.cpp
    src/readprofile.cpp
    src/startup.cpp
    PARENT_SCOPE
)
set(COMMON_HEADER_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PARENT_SCOPE
)

message(STATUS "Common header paths: ${COMMON_HEADER_PATHS}")
