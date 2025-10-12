cmake_minimum_required(VERSION 3.19)

set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/read_profile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/local_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key_channel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key_code.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/state_machine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util.cpp

    # For some reason it breaks without explicitly putting it here. 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/local_server.h
    PARENT_SCOPE
)
set(COMMON_HEADER_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PARENT_SCOPE
)
