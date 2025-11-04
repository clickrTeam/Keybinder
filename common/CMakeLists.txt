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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generic_indicator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key_counter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings.cpp

    # For some reason any header of a QObject needs to be here
    ${CMAKE_CURRENT_SOURCE_DIR}/include/local_server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/generic_indicator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/key_counter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/settings.h
    PARENT_SCOPE
)
set(COMMON_HEADER_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PARENT_SCOPE
)
