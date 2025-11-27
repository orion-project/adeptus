cmake_minimum_required(VERSION 3.16)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Remove per-configuration suffix from output directory added by VSCode
# https://stackoverflow.com/questions/47175912/using-cmake-how-to-stop-the-debug-and-release-subdirectories
# https://cmake.org/cmake/help/v3.4/prop_tgt/RUNTIME_OUTPUT_DIRECTORY.html#prop_tgt:RUNTIME_OUTPUT_DIRECTORY
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_SOURCE_DIR}/bin>)

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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Network Widgets)
find_package(Qt6 REQUIRED COMPONENTS Network Widgets Sql)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app.rc
    src/aboutwindow.cpp
    src/appearance.cpp
    src/browsercommands.cpp
    src/bugeditor.cpp
    src/bughistory.cpp
    src/bugitemdelegate.cpp
    src/bugmanager.cpp
    src/bugsolver.cpp
    src/bugtypes.cpp
    src/db/db.cpp
    src/db/historymanager.cpp
    src/db/issuemanager.cpp
    src/db/relationmanager.cpp
    src/db/sqlhelpers.cpp
    src/dicteditor.cpp
    src/filterpanel.cpp
    src/ImageViewWindow.cpp
    src/issuetable.cpp
    src/issuetextedit.cpp
    src/issuetextview.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/markdown.cpp
    src/markdowneditor.cpp
    src/operations.cpp
    src/preferences.cpp
    src/prefseditor.cpp
    src/startpage.cpp
)

qt_add_resources(PROJECT_SOURCES src/app.qrc)

add_subdirectory(orion)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    orion
    Qt::Widgets
    Qt::Sql
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
