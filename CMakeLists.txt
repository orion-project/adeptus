cmake_minimum_required(VERSION 3.5)

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

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/app.rc
    aboutwindow.cpp
    appearance.cpp
    browsercommands.cpp
    bugeditor.cpp
    bughistory.cpp
    bugitemdelegate.cpp
    bugmanager.cpp
    bugsolver.cpp
    bugtypes.cpp
    db/db.cpp
    db/historymanager.cpp
    db/issuemanager.cpp
    db/relationmanager.cpp
    db/sqlhelpers.cpp
    dicteditor.cpp
    filterpanel.cpp
    ImageViewWindow.cpp
    issuetable.cpp
    issuetextedit.cpp
    issuetextview.cpp
    main.cpp
    mainwindow.cpp
    markdown.cpp
    markdowneditor.cpp
    operations.cpp
    preferences.cpp
    prefseditor.cpp
    startpage.cpp
)

qt_add_resources(PROJECT_SOURCES images.qrc)

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
