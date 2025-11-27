#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("orion-project.org");
    app.setApplicationName("Adeptus");

    MainWindow w;
    w.show();

    return app.exec();
}
