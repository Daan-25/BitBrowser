#pragma once

#include <QMainWindow>
#include <QUrl>

class QTabWidget;
class QWebEngineView;
class QLineEdit;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUi();
    void connectSignals();

    QWebEngineView* createTab(const QUrl &url = QUrl("https://www.google.com"));
    QWebEngineView* currentView() const;

    void navigateToAddressBar();
    void syncAddressBarWithCurrentTab();
    void closeCurrentTab();
    void closeTabByIndex(int index);

private:
    QTabWidget *tabs = nullptr;
    QLineEdit *addressBar = nullptr;

    QAction *backAction = nullptr;
    QAction *forwardAction = nullptr;
    QAction *reloadAction = nullptr;
    QAction *homeAction = nullptr;
    QAction *newTabAction = nullptr;
    QAction *closeTabAction = nullptr;
};
