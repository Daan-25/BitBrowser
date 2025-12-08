#pragma once

#include <QDialog>

class QWebEngineProfile;
class QWebEngineExtensionManager;
class QWebEngineExtensionInfo;

class QLabel;
class QPushButton;

class ExtensionManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExtensionManagerDialog(QWebEngineProfile *profile, QWidget *parent = nullptr);

private slots:
    void installUnpacked();
    void installZip();
    void loadTemporary();

    void onInstallFinished(const QWebEngineExtensionInfo &info);
    void onLoadFinished(const QWebEngineExtensionInfo &info);

private:
    void setupUi();
    void setStatus(const QString &text);

private:
    QWebEngineProfile *m_profile = nullptr; // non-owning
    QWebEngineExtensionManager *m_mgr = nullptr;

    QLabel *m_status = nullptr;

    QPushButton *m_installUnpackedBtn = nullptr;
    QPushButton *m_installZipBtn = nullptr;
    QPushButton *m_loadTempBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
};
