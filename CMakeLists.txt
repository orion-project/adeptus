cmake_minimum_required(VERSION 3.16)

# Use Qt provided by vcpkg. Configure project as 
# cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DUSE_VCPKG_QT=ON
# when none of these options set, it's supposed that Qt is installed globally by its installer
# and all other dependecies also instlled globally via `vcpkg install` from root `vcpkg.json`
set(USE_VCPKG_QT OFF CACHE BOOL "Build with Qt installed via vcpkg")

if(USE_VCPKG_QT)
    set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_qt")
endif()

# Enable CMake's explicit MSVC runtime selection before project().
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# vcpkg's x64-windows-static triplet builds the MSVC CRT with /MT.
# Match it so application objects are compatible with static Qt,
# while shared builds retain CMake's default /MD runtime.
if(VCPKG_TARGET_TRIPLET STREQUAL "x64-windows-static")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

file(READ "release/version.txt" APP_VER_FULL)

project(adeptus VERSION ${APP_VER_FULL} LANGUAGES CXX)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP APP_VER_YEAR "%Y")

string(REPLACE "." ";" APP_VER_LIST ${APP_VER_FULL})
list(GET APP_VER_LIST 0 APP_VER_MAJOR)
list(GET APP_VER_LIST 1 APP_VER_MINOR)
list(GET APP_VER_LIST 2 APP_VER_PATCH)

configure_file(
    release/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable MSVC warnings about non-standard STL extensions globally
if(MSVC)
    add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
    add_compile_options(/wd4043)  # Disable STL4043 warning about stdext::checked_array_iterator
endif()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Network Widgets Sql Svg)
find_package(Qt6 REQUIRED COMPONENTS Network Widgets Sql Svg)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    src/app.rc
    src/app.qrc
    src/aboutwindow.cpp
    src/appearance.cpp
    src/browsercommands.cpp
    src/bugeditor.cpp
    src/bugitemdelegate.cpp
    src/bugmanager.cpp
    src/bugsolver.cpp
    src/bugtypes.cpp
    src/db/Db.cpp
    src/db/Dicts.cpp
    src/db/historymanager.cpp
    src/db/issuemanager.cpp
    src/db/Relations.cpp
    src/db/Settings.cpp
    src/db/SqlHelpers.cpp
    src/pages/HistoryPage.cpp
    src/pages/TablePage.cpp
    src/dicteditor.cpp
    src/filterpanel.cpp
    src/ImageViewWindow.cpp
    src/issuetextedit.cpp
    src/issuetextview.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/markdown.cpp
    src/markdowneditor.cpp
    src/operations.cpp
    src/Preferences.cpp
)

add_subdirectory(orion)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    orion
    Qt::Widgets
    Qt::Sql
    Qt::Svg
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.orion-project.adeptus
    MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VER_FULL}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BUILD_DATE="${BUILD_DATE}"
    APP_VER_MAJOR=${APP_VER_MAJOR}
    APP_VER_MINOR=${APP_VER_MINOR}
    APP_VER_PATCH=${APP_VER_PATCH}
    APP_VER_YEAR=${APP_VER_YEAR}
    QT_USE_QSTRINGBUILDER
)

qt_finalize_executable(${PROJECT_NAME})
