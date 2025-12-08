#include "extensionspage.h"

#include <QWebEngineProfile>
#include <QWebEngineExtensionManager>
#include <QWebEngineExtensionInfo>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <QFileDialog>
#include <QMessageBox>

static constexpr bool kQt6101 =
    (QT_VERSION == QT_VERSION_CHECK(6, 10, 1));

ExtensionsPage::ExtensionsPage(QWebEngineProfile *profile, QWidget *parent)
    : QWidget(parent)
    , m_profile(profile)
{
    setupUi();

    if (!m_profile) {
        setStatus("No profile provided.");
        return;
    }

    m_mgr = m_profile->extensionManager();
    if (!m_mgr) {
        setStatus("Extension manager not available.");
        return;
    }

    connect(m_mgr, &QWebEngineExtensionManager::installFinished,
            this, &ExtensionsPage::onInstallFinished);
    connect(m_mgr, &QWebEngineExtensionManager::loadFinished,
            this, &ExtensionsPage::onLoadFinished);

    applyQt6101GuardUi();
}

void ExtensionsPage::setupUi()
{
    auto *root = new QVBoxLayout(this);

    m_title = new QLabel("Extensions", this);
    QFont f = m_title->font();
    f.setPointSize(f.pointSize() + 2);
    f.setBold(true);
    m_title->setFont(f);

    m_warning = new QLabel(this);
    m_warning->setWordWrap(true);
    m_warning->hide();

    m_status = new QLabel(this);
    m_status->setWordWrap(true);

    root->addWidget(m_title);
    root->addWidget(m_warning);
    root->addWidget(m_status);

    auto *row = new QHBoxLayout();

    m_installUnpackedBtn = new QPushButton("Install unpacked...", this);
    m_installZipBtn = new QPushButton("Install from .zip...", this);
    m_loadTempBtn = new QPushButton("Load temporary...", this);

    row->addWidget(m_installUnpackedBtn);
    row->addWidget(m_installZipBtn);
    row->addWidget(m_loadTempBtn);
    row->addStretch();

    root->addLayout(row);
    root->addStretch();

    connect(m_installUnpackedBtn, &QPushButton::clicked, this, &ExtensionsPage::installUnpacked);
    connect(m_installZipBtn, &QPushButton::clicked, this, &ExtensionsPage::installZip);
    connect(m_loadTempBtn, &QPushButton::clicked, this, &ExtensionsPage::loadTemporary);
}

void ExtensionsPage::applyQt6101GuardUi()
{
    if (!kQt6101) {
        setStatus("Ready to install/load Manifest V3 extensions.");
        return;
    }

    m_warning->setText(
        "⚠ Extensions are disabled on <b>Qt 6.10.1</b> to prevent crashes.<br>"
        "Install or load is known to be unstable in this patch.<br><br>"
        "To test extensions, switch your kit to <b>Qt 6.10.0</b> (or newer stable patch)."
        );
    m_warning->setStyleSheet(
        "QLabel {"
        " background: #2b1d00;"
        " color: #ffd08a;"
        " padding: 10px;"
        " border-radius: 6px;"
        "}"
        );
    m_warning->show();

    m_installUnpackedBtn->setEnabled(false);
    m_installZipBtn->setEnabled(false);
    m_loadTempBtn->setEnabled(false);

    setStatus("Extensions disabled for safety on Qt 6.10.1.");
}

void ExtensionsPage::setStatus(const QString &text)
{
    if (m_status) m_status->setText(text);
}

void ExtensionsPage::installUnpacked()
{
    if (!m_mgr) return;
    if (kQt6101) return;

    const QString dir = QFileDialog::getExistingDirectory(
        this, "Select extension folder (contains manifest.json)");

    if (dir.isEmpty()) return;

    setStatus("Installing unpacked extension...");
    m_mgr->installExtension(dir);
}

void ExtensionsPage::installZip()
{
    if (!m_mgr) return;
    if (kQt6101) return;

    const QString zip = QFileDialog::getOpenFileName(
        this, "Select extension .zip", QString(), "Zip (*.zip)");

    if (zip.isEmpty()) return;

    setStatus("Installing .zip extension...");
    m_mgr->installExtension(zip);
}

void ExtensionsPage::loadTemporary()
{
    if (!m_mgr) return;
    if (kQt6101) return;

    const QString dir = QFileDialog::getExistingDirectory(
        this, "Select extension folder (temporary load)");

    if (dir.isEmpty()) return;

    setStatus("Loading temporary extension...");
    m_mgr->loadExtension(dir);
}

void ExtensionsPage::onInstallFinished(const QWebEngineExtensionInfo &info)
{
    if (kQt6101) return;

    if (!info.error().isEmpty()) {
        setStatus("Install failed: " + info.error());
        QMessageBox::warning(this, "Extension install failed", info.error());
        return;
    }

    if (m_mgr) m_mgr->setExtensionEnabled(info, true);
    setStatus("Installed and enabled: " + info.name());
}

void ExtensionsPage::onLoadFinished(const QWebEngineExtensionInfo &info)
{
    if (kQt6101) return;

    if (!info.error().isEmpty()) {
        setStatus("Load failed: " + info.error());
        QMessageBox::warning(this, "Extension load failed", info.error());
        return;
    }

    if (m_mgr) m_mgr->setExtensionEnabled(info, true);
    setStatus("Loaded temporary and enabled: " + info.name());
}
