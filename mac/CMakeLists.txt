set(PLATFORM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/daemon.cpp
    PARENT_SCOPE
)

set(PLATFORM_HEADER_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PARENT_SCOPE
)

set(PLATFORM_LIBS 
  "-framework IOKit" 
  "-framework CoreFoundation"
    PARENT_SCOPE
)
