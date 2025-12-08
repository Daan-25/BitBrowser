#pragma once

#include <QWidget>

class QWebEngineProfile;
class QWebEngineExtensionManager;
class QWebEngineExtensionInfo;

class QLabel;
class QPushButton;

class ExtensionsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ExtensionsPage(QWebEngineProfile *profile, QWidget *parent = nullptr);

private slots:
    void installUnpacked();
    void installZip();
    void loadTemporary();

    void onInstallFinished(const QWebEngineExtensionInfo &info);
    void onLoadFinished(const QWebEngineExtensionInfo &info);

private:
    void setupUi();
    void setStatus(const QString &text);
    void applyQt6101GuardUi();

private:
    QWebEngineProfile *m_profile = nullptr; // non-owning
    QWebEngineExtensionManager *m_mgr = nullptr;

    QLabel *m_title = nullptr;
    QLabel *m_warning = nullptr;
    QLabel *m_status = nullptr;

    QPushButton *m_installUnpackedBtn = nullptr;
    QPushButton *m_installZipBtn = nullptr;
    QPushButton *m_loadTempBtn = nullptr;
};