cmake_minimum_required(VERSION 3.19)
project(Keybinder LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Keybinder
    WIN32 MACOSX_BUNDLE
    main.cpp
    win/deamon.cpp win/deamon.h
    startup.h startup.cpp
    profileInterpreter/readprofile.h profileInterpreter/readprofile.cpp
    profileInterpreter/profile.h
    ubuntu/learning/SimpleRemap.cpp
)

target_link_libraries(Keybinder
    PRIVATE
        Qt::Core
        Qt::Widgets
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(Keybinder PRIVATE evdev)
endif()

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
