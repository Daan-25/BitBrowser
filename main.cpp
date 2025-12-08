#include <QApplication>
#include <QCoreApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("BitBrowser");
    QCoreApplication::setOrganizationDomain("bitbrowser.local");
    QCoreApplication::setApplicationName("BitBrowser");

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}
