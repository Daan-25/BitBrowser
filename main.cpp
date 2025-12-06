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

    // Stabiele app-identiteit voor consistente AppData paden
    QCoreApplication::setOrganizationName("Daan");
    QCoreApplication::setApplicationName("BitBrowser");

    auto *profile = QWebEngineProfile::defaultProfile();

    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/webengine";

    // Maak mappen aan
    QDir().mkpath(base);
    QDir().mkpath(base + "/cache");

    // Zet paden (gebruik base als storage root)
    profile->setPersistentStoragePath(base);
    profile->setCachePath(base + "/cache");

    // Forceer persistent gedrag
    profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    // Debug: check waar hij echt schrijft
    qDebug() << "AppDataLocation:" << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    qDebug() << "PersistentStoragePath:" << profile->persistentStoragePath();
    qDebug() << "CachePath:" << profile->cachePath();

    MainWindow w;
    w.show();

    return app.exec();
}
