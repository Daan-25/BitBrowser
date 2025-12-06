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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUi();
    void connectSignals();

    QWebEngineView* createTab(const QUrl &url = QUrl("https://www.google.com"));
    QWebEngineView* currentView() const;

    void navigateToAddressBar();
    void syncAddressBarWithCurrentTab();
    void closeCurrentTab();
    void closeTabByIndex(int index);

    void showPageSource(class QWebEngineView *view);

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

private:
    QTabWidget *tabs = nullptr;
    QLineEdit *addressBar = nullptr;

    QAction *backAction = nullptr;
    QAction *forwardAction = nullptr;
    QAction *reloadAction = nullptr;
    QAction *homeAction = nullptr;
    QAction *newTabAction = nullptr;
    QAction *closeTabAction = nullptr;

    QAction *historyAction = nullptr;
    QAction *clearHistoryAction = nullptr;
    QAction *clearSessionAction = nullptr;

    QVector<HistoryEntry> history;

    QAction *bookmarkStarAction = nullptr;
    QAction *showBookmarksAction = nullptr;
    QAction *clearBookmarksAction = nullptr;

    QVector<BookmarkEntry> bookmarks;

    QMenu *bookmarksMenu = nullptr;
    QAction *bookmarksSeparator = nullptr;
    QVector<QAction*> bookmarkItemActions;

private:
    QWebEngineProfile *browserProfile = nullptr;
};
