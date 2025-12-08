#include "mainwindow.h"
#include "extensionmanagerdialog.h"
#include "extensionspage.h"

#include <QTabWidget>
#include <QToolBar>
#include <QLineEdit>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QMenuBar>
#include <QMenu>
#include <QCloseEvent>

#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEnginePage>

#include <QWebEngineExtensionManager>
#include <QWebEngineExtensionInfo>

#include <QFileDialog>
#include <QMessageBox>

#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDateTime>

#include <QDialog>
#include <QVBoxLayout>
#include <QPlainTextEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // ✅ Eigen persistente profile (NIET defaultProfile)
    browserProfile = new QWebEngineProfile("BitBrowserProfile", this);

    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/webengine";

    QDir().mkpath(base);

    browserProfile->setPersistentStoragePath(base + "/storage");
    browserProfile->setCachePath(base + "/cache");
    browserProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    browserProfile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    // Extension manager
    extensionManager = browserProfile->extensionManager();
    if (extensionManager) {
        connect(extensionManager, &QWebEngineExtensionManager::installFinished,
                this, &MainWindow::onExtensionInstallFinished);
        connect(extensionManager, &QWebEngineExtensionManager::loadFinished,
                this, &MainWindow::onExtensionLoadFinished);
    }

    loadHistory();
    loadBookmarks();
    rebuildBookmarksMenu();

    connectSignals();

    loadSession();

    if (tabs->count() == 0) {
        createTab(QUrl("https://www.google.com"));
    }

    syncAddressBarWithCurrentTab();
    updateBookmarkStar();
}

void MainWindow::setupUi()
{
    setWindowTitle("BitBrowser");
    resize(1200, 800);

    tabs = new QTabWidget(this);
    tabs->setDocumentMode(true);
    tabs->setTabsClosable(true);
    tabs->setMovable(true);
    setCentralWidget(tabs);

    auto *toolbar = addToolBar("Navigation");
    toolbar->setMovable(false);

    backAction = toolbar->addAction("←");
    forwardAction = toolbar->addAction("→");
    reloadAction = toolbar->addAction("⟳");
    homeAction = toolbar->addAction("🏠");

    toolbar->addSeparator();

    newTabAction = toolbar->addAction("+");
    closeTabAction = toolbar->addAction("✕");

    toolbar->addSeparator();

    bookmarkStarAction = toolbar->addAction("☆");
    bookmarkStarAction->setToolTip("Toggle bookmark for current page");

    addressBar = new QLineEdit(this);
    addressBar->setPlaceholderText("Type a URL and press Enter...");
    toolbar->addWidget(addressBar);

    // History menu
    auto *historyMenu = menuBar()->addMenu("History");
    historyAction = historyMenu->addAction("Show History");
    clearHistoryAction = historyMenu->addAction("Clear History");
    historyAction->setShortcut(QKeySequence("Ctrl+H"));

    auto *sessionMenu = historyMenu->addMenu("Session");
    clearSessionAction = sessionMenu->addAction("Clear Saved Session");

    // Bookmarks menu
    bookmarksMenu = menuBar()->addMenu("Bookmarks");
    showBookmarksAction = bookmarksMenu->addAction("Show Bookmarks");
    clearBookmarksAction = bookmarksMenu->addAction("Clear Bookmarks");
    showBookmarksAction->setShortcut(QKeySequence("Ctrl+Shift+B"));
    bookmarksSeparator = bookmarksMenu->addSeparator();

    // Extensions menu
    setupExtensionsMenu();

    // Shortcuts
    new QShortcut(QKeySequence::AddTab, this, [this]() { createTab(); });
    new QShortcut(QKeySequence::Close, this, [this]() { closeCurrentTab(); });
    new QShortcut(QKeySequence("Ctrl+L"), this, [this]() {
        addressBar->setFocus();
        addressBar->selectAll();
    });
    new QShortcut(QKeySequence("Ctrl+R"), this, [this]() {
        if (auto *v = currentView()) v->reload();
    });
}

void MainWindow::setupExtensionsMenu()
{
    extensionsMenu = menuBar()->addMenu("Extensions");

    openInternalExtensionsAction = extensionsMenu->addAction("Open bit://extensions");
    extensionsMenu->addSeparator();

    manageExtensionsAction = extensionsMenu->addAction("Manage Extensions...");
    extensionsMenu->addSeparator();

    installUnpackedAction = extensionsMenu->addAction("Install unpacked...");
    installZipAction = extensionsMenu->addAction("Install from .zip...");
    extensionsMenu->addSeparator();
    loadTempAction = extensionsMenu->addAction("Load unpacked (temporary)...");
}

void MainWindow::connectSignals()
{
    connect(backAction, &QAction::triggered, this, [this]() {
        if (auto *v = currentView()) v->back();
    });
    connect(forwardAction, &QAction::triggered, this, [this]() {
        if (auto *v = currentView()) v->forward();
    });
    connect(reloadAction, &QAction::triggered, this, [this]() {
        if (auto *v = currentView()) v->reload();
    });
    connect(homeAction, &QAction::triggered, this, [this]() {
        if (auto *v = currentView()) v->load(QUrl("https://www.google.com"));
    });

    connect(newTabAction, &QAction::triggered, this, [this]() { createTab(); });
    connect(closeTabAction, &QAction::triggered, this, [this]() { closeCurrentTab(); });

    connect(addressBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToAddressBar);

    connect(historyAction, &QAction::triggered, this, &MainWindow::showHistoryDialog);
    connect(clearHistoryAction, &QAction::triggered, this, &MainWindow::clearHistory);

    connect(clearSessionAction, &QAction::triggered, this, &MainWindow::clearSession);

    connect(bookmarkStarAction, &QAction::triggered, this, &MainWindow::toggleCurrentBookmark);
    connect(showBookmarksAction, &QAction::triggered, this, &MainWindow::showBookmarksDialog);
    connect(clearBookmarksAction, &QAction::triggered, this, &MainWindow::clearBookmarks);

    // Extensions
    if (openInternalExtensionsAction)
        connect(openInternalExtensionsAction, &QAction::triggered,
                this, &MainWindow::openExtensionsInternalPage);

    if (manageExtensionsAction)
        connect(manageExtensionsAction, &QAction::triggered,
                this, &MainWindow::showExtensionsManager);

    if (installUnpackedAction)
        connect(installUnpackedAction, &QAction::triggered,
                this, &MainWindow::installUnpackedExtension);

    if (installZipAction)
        connect(installZipAction, &QAction::triggered,
                this, &MainWindow::installZippedExtension);

    if (loadTempAction)
        connect(loadTempAction, &QAction::triggered,
                this, &MainWindow::loadUnpackedTempExtension);

    // Tabs
    connect(tabs, &QTabWidget::currentChanged, this, [this](int) {
        syncAddressBarWithCurrentTab();
        updateBookmarkStar();
    });

    connect(tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTabByIndex);
}

bool MainWindow::isExtensionsText(const QString &text) const
{
    const QString s = text.trimmed().toLower();
    return s == "bit://extensions" || s.startsWith("bit://extensions/");
}

void MainWindow::openExtensionsInternalPage()
{
    if (!browserProfile) {
        QMessageBox::warning(this, "Extensions", "Browser profile not initialized.");
        return;
    }

    // Al open?
    for (int i = 0; i < tabs->count(); ++i) {
        if (tabs->tabText(i) == "Extensions") {
            tabs->setCurrentIndex(i);
            addressBar->setText("bit://extensions");
            return;
        }
    }

    auto *page = new ExtensionsPage(browserProfile, tabs);
    int index = tabs->addTab(page, "Extensions");
    tabs->setCurrentIndex(index);

    addressBar->setText("bit://extensions");
}

QWebEngineView* MainWindow::createTab(const QUrl &url)
{
    const QString s = url.toString().trimmed();
    if (isExtensionsText(s)) {
        openExtensionsInternalPage();
        return nullptr;
    }

    auto *view = new QWebEngineView(tabs);

    // ✅ Gebruik altijd jouw persistente profile
    auto *page = new QWebEnginePage(browserProfile, view);
    view->setPage(page);

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QWidget::customContextMenuRequested, this, [this, view](const QPoint &pos) {
        QMenu menu;

        menu.addAction(view->page()->action(QWebEnginePage::Back));
        menu.addAction(view->page()->action(QWebEnginePage::Forward));
        menu.addAction(view->page()->action(QWebEnginePage::Reload));

        menu.addSeparator();

        menu.addAction(view->page()->action(QWebEnginePage::Copy));
        menu.addAction(view->page()->action(QWebEnginePage::SelectAll));

        menu.addSeparator();

        QAction *sourceAction = menu.addAction("View Page Source");
        connect(sourceAction, &QAction::triggered, this, [this, view]() {
            showPageSource(view);
        });

        menu.exec(view->mapToGlobal(pos));
    });

    int index = tabs->addTab(view, "New Tab");
    tabs->setCurrentIndex(index);

    connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) {
        int i = tabs->indexOf(view);
        if (i != -1)
            tabs->setTabText(i, title.isEmpty() ? "New Tab" : title.left(40));

        if (view == currentView()) {
            syncAddressBarWithCurrentTab();
            updateBookmarkStar();
            setWindowTitle((title.isEmpty() ? "BitBrowser" : title) + " - BitBrowser");
        }
    });

    connect(view, &QWebEngineView::urlChanged, this, [this, view](const QUrl &newUrl) {
        if (view == currentView()) {
            addressBar->setText(newUrl.toString());
            updateBookmarkStar();
        }
    });

    connect(view, &QWebEngineView::loadFinished, this, [this, view](bool ok) {
        if (!ok) return;
        const QUrl u = view->url();
        if (!u.isValid() || u.toString().isEmpty()) return;

        addHistoryEntry(u, view->title());
    });

    view->load(url);
    return view;
}

QWebEngineView* MainWindow::currentView() const
{
    return qobject_cast<QWebEngineView*>(tabs->currentWidget());
}

void MainWindow::navigateToAddressBar()
{
    const QString text = addressBar->text().trimmed();

    if (isExtensionsText(text)) {
        openExtensionsInternalPage();
        return;
    }

    const QUrl url = QUrl::fromUserInput(text);

    if (auto *view = currentView()) {
        view->load(url);
    } else {
        createTab(url);
    }
}

void MainWindow::syncAddressBarWithCurrentTab()
{
    auto *view = currentView();
    if (!view) {
        // interne tab actief
        return;
    }
    addressBar->setText(view->url().toString());
}

/* ---------------- Tabs close helpers ---------------- */

void MainWindow::closeCurrentTab()
{
    closeTabByIndex(tabs->currentIndex());
}

void MainWindow::closeTabByIndex(int index)
{
    if (index < 0) return;

    QWidget *w = tabs->widget(index);
    tabs->removeTab(index);
    delete w;

    if (tabs->count() == 0)
        createTab(QUrl("https://www.google.com"));
}

/* ---------------- History ---------------- */

QString MainWindow::historyFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/history.json";
}

void MainWindow::loadHistory()
{
    history.clear();

    QFile f(historyFilePath());
    if (!f.exists() || !f.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isArray()) return;

    const QJsonArray arr = doc.array();

    for (const auto &v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        HistoryEntry e;
        e.url = QUrl(o.value("url").toString());
        e.title = o.value("title").toString();
        e.visitedAt = QDateTime::fromString(o.value("visitedAt").toString(), Qt::ISODate);

        if (e.url.isValid())
            history.push_back(e);
    }
}

void MainWindow::saveHistory() const
{
    QJsonArray arr;

    for (const auto &e : history) {
        QJsonObject o;
        o["url"] = e.url.toString();
        o["title"] = e.title;
        o["visitedAt"] = e.visitedAt.toString(Qt::ISODate);
        arr.push_back(o);
    }

    QFile f(historyFilePath());
    if (!f.open(QIODevice::WriteOnly))
        return;

    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    f.close();
}

void MainWindow::addHistoryEntry(const QUrl &url, const QString &title)
{
    HistoryEntry e;
    e.url = url;
    e.title = title;
    e.visitedAt = QDateTime::currentDateTime();

    history.push_back(e);

    const int maxEntries = 3000;
    if (history.size() > maxEntries) {
        history.erase(history.begin(), history.begin() + (history.size() - maxEntries));
    }

    saveHistory();
}

void MainWindow::showHistoryDialog()
{
    HistoryDialog dlg(history, this);

    connect(&dlg, &HistoryDialog::openUrlRequested, this, [this](const QUrl &url) {
        if (auto *v = currentView()) v->load(url);
        else createTab(url);
    });

    connect(&dlg, &HistoryDialog::clearRequested, this, [this]() {
        clearHistory();
    });

    dlg.exec();
}

void MainWindow::clearHistory()
{
    history.clear();
    QFile::remove(historyFilePath());
    saveHistory();
}

/* ---------------- Session restore ---------------- */

QString MainWindow::sessionFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/session.json";
}

void MainWindow::loadSession()
{
    QFile f(sessionFilePath());
    if (!f.exists() || !f.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isArray()) return;

    const QJsonArray arr = doc.array();
    for (const auto &v : arr) {
        const QUrl url(v.toString());
        if (url.isValid() && !url.toString().isEmpty())
            createTab(url);
    }
}

void MainWindow::saveSession() const
{
    QJsonArray arr;

    for (int i = 0; i < tabs->count(); ++i) {
        auto *view = qobject_cast<QWebEngineView*>(tabs->widget(i));
        if (!view) continue; // interne tabs skippen

        const QUrl url = view->url();
        if (url.isValid() && !url.toString().isEmpty())
            arr.push_back(url.toString());
    }

    QFile f(sessionFilePath());
    if (!f.open(QIODevice::WriteOnly))
        return;

    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    f.close();
}

void MainWindow::clearSession()
{
    QFile::remove(sessionFilePath());

    while (tabs->count() > 0) {
        QWidget *w = tabs->widget(0);
        tabs->removeTab(0);
        delete w;
    }

    createTab(QUrl("https://www.google.com"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSession();
    QMainWindow::closeEvent(event);
}

/* ---------------- Bookmarks ---------------- */

QString MainWindow::bookmarksFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/bookmarks.json";
}

void MainWindow::loadBookmarks()
{
    bookmarks.clear();

    QFile f(bookmarksFilePath());
    if (!f.exists() || !f.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isArray()) return;

    const QJsonArray arr = doc.array();
    for (const auto &v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        BookmarkEntry e;
        e.url = QUrl(o.value("url").toString());
        e.title = o.value("title").toString();

        if (e.url.isValid())
            bookmarks.push_back(e);
    }
}

void MainWindow::saveBookmarks() const
{
    QJsonArray arr;

    for (const auto &e : bookmarks) {
        QJsonObject o;
        o["url"] = e.url.toString();
        o["title"] = e.title;
        arr.push_back(o);
    }

    QFile f(bookmarksFilePath());
    if (!f.open(QIODevice::WriteOnly))
        return;

    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    f.close();
}

bool MainWindow::isBookmarked(const QUrl &url) const
{
    const QString u = url.toString();
    for (const auto &b : bookmarks) {
        if (b.url.toString() == u)
            return true;
    }
    return false;
}

void MainWindow::addBookmark(const QUrl &url, const QString &title)
{
    if (!url.isValid() || url.toString().isEmpty()) return;
    if (isBookmarked(url)) return;

    BookmarkEntry e;
    e.url = url;
    e.title = title;

    bookmarks.push_back(e);
    saveBookmarks();
    rebuildBookmarksMenu();
    updateBookmarkStar();
}

void MainWindow::removeBookmark(const QUrl &url)
{
    const QString u = url.toString();

    for (int i = 0; i < bookmarks.size(); ++i) {
        if (bookmarks[i].url.toString() == u) {
            bookmarks.removeAt(i);
            saveBookmarks();
            rebuildBookmarksMenu();
            updateBookmarkStar();
            return;
        }
    }
}

void MainWindow::toggleCurrentBookmark()
{
    auto *view = currentView();
    if (!view) return;

    const QUrl url = view->url();
    if (!url.isValid() || url.toString().isEmpty()) return;

    if (isBookmarked(url))
        removeBookmark(url);
    else
        addBookmark(url, view->title());
}

void MainWindow::updateBookmarkStar()
{
    if (!bookmarkStarAction) return;

    auto *view = currentView();
    if (!view) {
        bookmarkStarAction->setText("☆");
        return;
    }

    bookmarkStarAction->setText(isBookmarked(view->url()) ? "★" : "☆");
}

void MainWindow::showBookmarksDialog()
{
    BookmarksDialog dlg(bookmarks, this);

    connect(&dlg, &BookmarksDialog::openUrlRequested, this, [this](const QUrl &url) {
        if (auto *v = currentView()) v->load(url);
        else createTab(url);
    });

    connect(&dlg, &BookmarksDialog::removeUrlRequested, this, [this](const QUrl &url) {
        removeBookmark(url);
    });

    connect(&dlg, &BookmarksDialog::clearRequested, this, [this]() {
        clearBookmarks();
    });

    dlg.exec();
}

void MainWindow::clearBookmarks()
{
    bookmarks.clear();
    QFile::remove(bookmarksFilePath());
    saveBookmarks();
    rebuildBookmarksMenu();
    updateBookmarkStar();
}

void MainWindow::rebuildBookmarksMenu()
{
    if (!bookmarksMenu) return;

    for (auto *act : bookmarkItemActions) {
        bookmarksMenu->removeAction(act);
        delete act;
    }
    bookmarkItemActions.clear();

    for (const auto &b : bookmarks) {
        const QString title = b.title.isEmpty() ? b.url.toString() : b.title;

        QAction *act = new QAction(title, bookmarksMenu);
        act->setToolTip(b.url.toString());
        act->setData(b.url);

        connect(act, &QAction::triggered, this, [this, act]() {
            const QUrl url = act->data().toUrl();
            if (!url.isValid()) return;

            if (auto *v = currentView()) v->load(url);
            else createTab(url);
        });

        bookmarksMenu->addAction(act);
        bookmarkItemActions.push_back(act);
    }

    if (bookmarks.isEmpty()) {
        QAction *empty = new QAction("No bookmarks yet", bookmarksMenu);
        empty->setEnabled(false);
        bookmarksMenu->addAction(empty);
        bookmarkItemActions.push_back(empty);
    }
}

/* ---------------- Page Source ---------------- */

void MainWindow::showPageSource(QWebEngineView *view)
{
    if (!view) return;

    view->page()->toHtml([this, view](const QString &html) {
        auto *dlg = new QDialog(this);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setWindowTitle("Page Source - " + view->url().toString());
        dlg->resize(900, 600);

        auto *layout = new QVBoxLayout(dlg);
        auto *editor = new QPlainTextEdit(dlg);
        editor->setReadOnly(true);
        editor->setPlainText(html);

        layout->addWidget(editor);

        dlg->show();
    });
}

/* ---------------- Extensions Menu Actions ---------------- */

void MainWindow::showExtensionsManager()
{
    if (!browserProfile) {
        QMessageBox::warning(this, "Extensions", "Browser profile not initialized.");
        return;
    }

    ExtensionManagerDialog dlg(browserProfile, this);
    dlg.exec();
}

void MainWindow::installUnpackedExtension()
{
    if (!extensionManager) {
        QMessageBox::warning(this, "Extensions", "Extension manager not available.");
        return;
    }

    const QString dir = QFileDialog::getExistingDirectory(
        this, "Select extension folder (contains manifest.json)");

    if (dir.isEmpty()) return;

    extensionManager->installExtension(dir);
}

void MainWindow::installZippedExtension()
{
    if (!extensionManager) {
        QMessageBox::warning(this, "Extensions", "Extension manager not available.");
        return;
    }

    const QString zip = QFileDialog::getOpenFileName(
        this, "Select extension .zip", QString(), "Zip (*.zip)");

    if (zip.isEmpty()) return;

    extensionManager->installExtension(zip);
}

void MainWindow::loadUnpackedTempExtension()
{
    if (!extensionManager) {
        QMessageBox::warning(this, "Extensions", "Extension manager not available.");
        return;
    }

    const QString dir = QFileDialog::getExistingDirectory(
        this, "Select extension folder (temporary load)");

    if (dir.isEmpty()) return;

    extensionManager->loadExtension(dir);
}

void MainWindow::onExtensionInstallFinished(const QWebEngineExtensionInfo &info)
{
    if (!extensionManager) return;

    if (!info.error().isEmpty()) {
        QMessageBox::warning(this, "Extension install failed", info.error());
        return;
    }

    extensionManager->setExtensionEnabled(info, true);
}

void MainWindow::onExtensionLoadFinished(const QWebEngineExtensionInfo &info)
{
    if (!extensionManager) return;

    if (!info.error().isEmpty()) {
        QMessageBox::warning(this, "Extension load failed", info.error());
        return;
    }

    extensionManager->setExtensionEnabled(info, true);
}
