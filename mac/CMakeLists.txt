set(PLATFORM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/daemon.cpp
    ${CMAKE_SOURCE_DIR}/unix/src/signal_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key_map.cpp
    PARENT_SCOPE
)

set(PLATFORM_HEADER_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/Karabiner-DriverKit-VirtualHIDDevice/include/pqrs/karabiner/driverkit
    ${CMAKE_CURRENT_SOURCE_DIR}/external/Karabiner-DriverKit-VirtualHIDDevice/src/Daemon/vendor/include
    ${CMAKE_SOURCE_DIR}/unix/include
    PARENT_SCOPE
)

set(PLATFORM_LIBS 
  "-framework IOKit" 
  "-framework CoreFoundation"
    PARENT_SCOPE
)
