cmake_minimum_required(VERSION 3.19)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 20)
project(keybinder LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

# Conditionally add the platform-specific directory.
set(COMMON_SOURCES "")
set(COMMON_HEADER_PATHS "")
add_subdirectory(common)

set(PLATFORM_SOURCES "")
set(PLATFORM_HEADER_PATHS "")
set(PLATFORM_LIBS "")
if (WIN32)
    add_subdirectory(win)
elseif (APPLE)
    add_subdirectory(mac)
elseif (UNIX)
    add_subdirectory(linux)
endif()

qt_add_executable(keybinder
    WIN32 
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(keybinder  PRIVATE  
    ${COMMON_HEADER_PATHS} 
    ${PLATFORM_HEADER_PATHS} 
)

target_link_libraries(keybinder
    PRIVATE
        Qt::Core
        Qt::Network
        ${PLATFORM_LIBS}
)
