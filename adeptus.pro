#-------------------------------------------------
#
# Project created by QtCreator 2012-07-04T23:25:18
#
#-------------------------------------------------

QT += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32: TARGET = Adeptus
else: TARGET = adeptus
TEMPLATE = app
DESTDIR = $$_PRO_FILE_PWD_/bin

include($$_PRO_FILE_PWD_"/orion/orion.pri")

#------------------------------------------------------------
# Version information

include(release/version.pri)
DEFINES += "APP_VER_MAJOR=$$APP_VER_MAJOR"
DEFINES += "APP_VER_MINOR=$$APP_VER_MINOR"
DEFINES += "APP_VER_PATCH=$$APP_VER_PATCH"
DEFINES += "APP_VER_CODENAME=\"\\\"$$APP_VER_CODENAME\\\"\""

win32 {
    DEFINES += "APP_BUILDDATE=\"\\\"$$system(date /T)\\\"\""
    DEFINES += "APP_BUILDTIME=\"\\\"$$system(time /T)\\\"\""
}
else {
    DEFINES += "APP_BUILDDATE=\"\\\"$$system(date '+%F')\\\"\""
    DEFINES += "APP_BUILDTIME=\"\\\"$$system(date '+%T')\\\"\""
}

win32: RC_FILE = src/app.rc

#------------------------------------------------------------

SOURCES += src/main.cpp\
    src/mainwindow.cpp \
    src/operations.cpp \
    src/preferences.cpp \
    src/db/db.cpp \
    src/db/historymanager.cpp \
    src/db/issuemanager.cpp \
    src/db/relationmanager.cpp \
    src/db/sqlhelpers.cpp \
    src/db_old/bugmanager.cpp \
    src/db_old/bugtypes.cpp \
    src/dialogs/aboutwindow.cpp \
    src/dialogs/bugeditor.cpp \
    src/dialogs/bugsolver.cpp \
    src/dialogs/dicteditor.cpp \
    src/dialogs/imageviewwindow.cpp \
    src/dialogs/prefseditor.cpp \
    src/pages/bughistory.cpp \
    src/pages/issuetable.cpp \
    src/pages/startpage.cpp \
    src/utils/appearance.cpp \
    src/utils/browsercommands.cpp \
    src/utils/bugitemdelegate.cpp \
    src/utils/markdown.cpp \
    src/widgets/filterpanel.cpp \
    src/widgets/issuetextview.cpp \
    src/widgets/markdowneditor.cpp

HEADERS += \
    src/mainwindow.h \
    src/operations.h \
    src/preferences.h \
    src/db/db.h \
    src/db/historymanager.h \
    src/db/issuemanager.h \
    src/db/relationmanager.h \
    src/db/sqlhelpers.h \
    src/db_old/bugmanager.h \
    src/db_old/bugtypes.h \
    src/dialogs/aboutwindow.h \
    src/dialogs/bugeditor.h \
    src/dialogs/bugsolver.h \
    src/dialogs/dicteditor.h \
    src/dialogs/imageviewwindow.h \
    src/dialogs/prefseditor.h \
    src/pages/bughistory.h \
    src/pages/issuetable.h \
    src/pages/startpage.h \
    src/utils/appearance.h \
    src/utils/browsercommands.h \
    src/utils/bugitemdelegate.h \
    src/utils/markdown.h \
    src/widgets/filterpanel.h \
    src/widgets/issuetextview.h \
    src/widgets/markdowneditor.h \

RESOURCES = src/images.qrc
