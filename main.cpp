#include <QApplication>

#include "mainwindow.h"
#include "tools/OriDebug.h"

int main(int argc, char *argv[])
{
    Ori::Debug::installMessageHandler();

    QApplication app(argc, argv);

    app.setOrganizationName("orion-project.org");
    app.setApplicationName("Adeptus");

    MainWindow w;
    w.show();

    return app.exec();
}
