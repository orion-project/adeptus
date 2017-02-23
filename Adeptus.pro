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

exists("../orion/orion.pri") { ORION="../orion/" }
else: exists("../Orion.Qt/orion.pri") { ORION="../Orion.Qt/" }
else: exists("../orion-qt/orion.pri") { ORION="../orion-qt/" }
include($$ORION"orion.pri")

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
    bugoperations.cpp \
    bugtypes.cpp \
    ImageViewWindow.cpp \
    SqlBugProvider.cpp \
    SqlHelpers.cpp \
    markdown.cpp \
    browsercommands.cpp

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
    bugoperations.h \
    bugtypes.h \
    ImageViewWindow.h \
    SqlBugProvider.h \
    SqlHelpers.h \
    markdown.h \
    browsercommands.h

RESOURCES = images.qrc

DEFINES += "APP_VER_MAJOR=1"
DEFINES += "APP_VER_MINOR=0"
DEFINES += "APP_VER_BUILD=0"
DEFINES += "SVN_REV=\"\\\"$$system(svnversion -n)\\\"\""

win32 {
    DEFINES += "BUILDDATE=\"\\\"$$system(date /T)\\\"\""
    DEFINES += "BUILDTIME=\"\\\"$$system(time /T)\\\"\""
}
else {
    DEFINES += "BUILDDATE=\"\\\"$$system(date '+%F')\\\"\""
    DEFINES += "BUILDTIME=\"\\\"$$system(date '+%T')\\\"\""
}

win32 {
    RC_FILE = app.rc
    WCREV = "c:\\Program Files\\TortoiseSVN\\bin\\SubWCRev.exe"
    system(\"$$WCREV\" $$_PRO_FILE_PWD_ app.template.rc app.rc -f)
}
