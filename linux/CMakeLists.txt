set(PLATFORM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/daemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux_configure.cpp
    ${CMAKE_SOURCE_DIR}/unix/src/signal_handler.cpp
    PARENT_SCOPE
)

set(PLATFORM_HEADER_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/unix/include
    PARENT_SCOPE
)

set(PLATFORM_LIBS
    evdev
    PARENT_SCOPE
)

