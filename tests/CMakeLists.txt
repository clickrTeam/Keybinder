cmake_policy(SET CMP0135 NEW)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# ---- Test binary (only common sources/headers) ----
add_executable(keybinder_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp
    ${CMAKE_SOURCE_DIR}/common/src/key_channel.cpp 
    ${CMAKE_SOURCE_DIR}/common/src/key_code.cpp
    ${CMAKE_SOURCE_DIR}/common/src/logger.cpp
    ${CMAKE_SOURCE_DIR}/common/src/mapper.cpp
    ${CMAKE_SOURCE_DIR}/common/src/read_profile.cpp
    ${CMAKE_SOURCE_DIR}/common/src/generic_indicator.cpp
    ${CMAKE_SOURCE_DIR}/common/include/generic_indicator.h
    ${CMAKE_SOURCE_DIR}/common/src/tray.cpp
    ${CMAKE_SOURCE_DIR}/common/include/tray.h
)
set_target_properties(keybinder_tests PROPERTIES AUTOMOC ON)

target_include_directories(keybinder_tests PRIVATE
    ${COMMON_HEADER_PATHS}
    include/
)

target_link_libraries(keybinder_tests PRIVATE
    GTest::gtest_main
    Qt::Core
    Qt::Network
    Qt::Widgets
)

add_test(NAME keybinder_tests COMMAND keybinder_tests)
