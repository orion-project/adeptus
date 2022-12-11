#-------------------------------------------------
#
# Project created by QtCreator 2012-07-04T23:25:18
#
#-------------------------------------------------

QT += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

win32: TARGET = Adeptus
else: TARGET = adeptus
TEMPLATE = app
DESTDIR = $$_PRO_FILE_PWD_/bin

include($$_PRO_FILE_PWD_"/orion/orion.pri")

#------------------------------------------------------------
# Version information

include(release/version.pri)

win32: RC_FILE = app.rc

#------------------------------------------------------------

SOURCES += main.cpp\
    mainwindow.cpp \
    bugmanager.cpp \
    dicteditor.cpp \
    bugeditor.cpp \
    bugsolver.cpp \
    bughistory.cpp \
    preferences.cpp \
    prefseditor.cpp \
    bugitemdelegate.cpp \
    issuetable.cpp \
    aboutwindow.cpp \
    startpage.cpp \
    filterpanel.cpp \
    appearance.cpp \
    bugtypes.cpp \
    ImageViewWindow.cpp \
    markdown.cpp \
    browsercommands.cpp \
    markdowneditor.cpp \
    issuetextview.cpp \
    db/sqlhelpers.cpp \
    db/historymanager.cpp \
    db/db.cpp \
    db/issuemanager.cpp \
    db/relationmanager.cpp \
    operations.cpp

HEADERS  += mainwindow.h \
    bugmanager.h \
    dicteditor.h \
    bugeditor.h \
    bugsolver.h \
    bughistory.h \
    preferences.h \
    prefseditor.h \
    bugitemdelegate.h \
    issuetable.h \
    aboutwindow.h \
    startpage.h \
    filterpanel.h \
    appearance.h \
    bugtypes.h \
    ImageViewWindow.h \
    markdown.h \
    browsercommands.h \
    markdowneditor.h \
    issuetextview.h \
    db/sqlhelpers.h \
    db/historymanager.h \
    db/db.h \
    db/issuemanager.h \
    db/relationmanager.h \
    operations.h

RESOURCES = images.qrc
