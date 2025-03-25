cmake_minimum_required(VERSION 3.19)
project(Keybinder LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(SOURCES "")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SOURCES
        win/deamon.cpp win/deamon.h
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SOURCES
        ""
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin") # macOS
    set(SOURCES
        ""
    )
endif()

qt_add_executable(Keybinder
    WIN32 MACOSX_BUNDLE
    main.cpp
    win/deamon.cpp win/deamon.h
    startup.h startup.cpp
    profileInterpreter/readprofile.h profileInterpreter/readprofile.cpp
    profileInterpreter/profile.h
    ${SOURCES}
)

target_link_libraries(Keybinder
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS Keybinder
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Keybinder
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
