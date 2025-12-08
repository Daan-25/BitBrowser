#pragma once

#include <QMainWindow>
#include <QVector>
#include <QUrl>

#include "historydialog.h"
#include "bookmarksdialog.h"

class QTabWidget;
class QWebEngineView;
class QLineEdit;
class QAction;
class QCloseEvent;
class QMenu;
class QWebEngineProfile;
class QWebEnginePage;
class ExtensionManagerDialog;
class ExtensionsPage;

// Extensions
class QWebEngineExtensionManager;
class QWebEngineExtensionInfo;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    // UI
    void setupUi();
    void connectSignals();
    void setupExtensionsMenu();

    // Tabs / navigation
    QWebEngineView* createTab(const QUrl &url = QUrl("https://www.google.com"));
    QWebEngineView* currentView() const;

    void navigateToAddressBar();
    void syncAddressBarWithCurrentTab();
    void closeCurrentTab();
    void closeTabByIndex(int index);

    void showPageSource(QWebEngineView *view);

    // Internal pages
    bool isExtensionsText(const QString &text) const;
    void openExtensionsInternalPage();

    // History
    void loadHistory();
    void saveHistory() const;
    QString historyFilePath() const;
    void addHistoryEntry(const QUrl &url, const QString &title);
    void showHistoryDialog();
    void clearHistory();

    // Session restore
    QString sessionFilePath() const;
    void loadSession();
    void saveSession() const;
    void clearSession();

    // Bookmarks
    QString bookmarksFilePath() const;
    void loadBookmarks();
    void saveBookmarks() const;
    bool isBookmarked(const QUrl &url) const;
    void addBookmark(const QUrl &url, const QString &title);
    void removeBookmark(const QUrl &url);
    void toggleCurrentBookmark();
    void updateBookmarkStar();
    void showBookmarksDialog();
    void clearBookmarks();
    void rebuildBookmarksMenu();

    // Extensions (menu actions)
    void showExtensionsManager();

private slots:
    void installUnpackedExtension();
    void installZippedExtension();
    void loadUnpackedTempExtension();

    void onExtensionInstallFinished(const QWebEngineExtensionInfo &info);
    void onExtensionLoadFinished(const QWebEngineExtensionInfo &info);

private:
    // Core widgets
    QTabWidget *tabs = nullptr;
    QLineEdit *addressBar = nullptr;

    // Toolbar actions
    QAction *backAction = nullptr;
    QAction *forwardAction = nullptr;
    QAction *reloadAction = nullptr;
    QAction *homeAction = nullptr;
    QAction *newTabAction = nullptr;
    QAction *closeTabAction = nullptr;

    // History actions/data
    QAction *historyAction = nullptr;
    QAction *clearHistoryAction = nullptr;
    QVector<HistoryEntry> history;

    // Session action
    QAction *clearSessionAction = nullptr;

    // Bookmarks
    QAction *bookmarkStarAction = nullptr;
    QAction *showBookmarksAction = nullptr;
    QAction *clearBookmarksAction = nullptr;
    QVector<BookmarkEntry> bookmarks;

    QMenu *bookmarksMenu = nullptr;
    QAction *bookmarksSeparator = nullptr;
    QVector<QAction*> bookmarkItemActions;

    // Extensions menu/actions
    QMenu *extensionsMenu = nullptr;

    QAction *openInternalExtensionsAction = nullptr;
    QAction *manageExtensionsAction = nullptr;
    QAction *installUnpackedAction = nullptr;
    QAction *installZipAction = nullptr;
    QAction *loadTempAction = nullptr;

    // Browser profile + extensions
    QWebEngineProfile *browserProfile = nullptr;            // owning
    QWebEngineExtensionManager *extensionManager = nullptr; // from profile
};
