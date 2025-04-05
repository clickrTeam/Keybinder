cmake_minimum_required(VERSION 3.19)

set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/readprofile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mapper.cpp
    PARENT_SCOPE
)
set(COMMON_HEADER_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PARENT_SCOPE
)
