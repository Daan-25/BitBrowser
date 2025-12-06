#include <QApplication>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include <QWebEngineProfile>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Bit");
    QCoreApplication::setApplicationName("BitBrowser");

    auto *profile = QWebEngineProfile::defaultProfile();

    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/webengine";

    QDir().mkpath(base);
    QDir().mkpath(base + "/cache");

    profile->setPersistentStoragePath(base);
    profile->setCachePath(base + "/cache");

    profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    qDebug() << "AppDataLocation:" << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    qDebug() << "PersistentStoragePath:" << profile->persistentStoragePath();
    qDebug() << "CachePath:" << profile->cachePath();

    MainWindow w;
    w.show();

    return app.exec();
}
