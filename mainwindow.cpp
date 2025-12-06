#include "mainwindow.h"

#include <QTabWidget>
#include <QToolBar>
#include <QLineEdit>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>

#include <QWebEngineView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    connectSignals();

    createTab(QUrl("https://www.google.com"));
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

    addressBar = new QLineEdit(this);
    addressBar->setPlaceholderText("Typ een URL en druk op Enter...");
    toolbar->addWidget(addressBar);

    new QShortcut(QKeySequence::AddTab, this, [this]() { createTab(); });
    new QShortcut(QKeySequence::Close, this, [this]() { closeCurrentTab(); });
    new QShortcut(QKeySequence("Ctrl+L"), this, [this]() { addressBar->setFocus(); addressBar->selectAll(); });
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

    connect(newTabAction, &QAction::triggered, this, [this]() {
        createTab();
    });

    connect(closeTabAction, &QAction::triggered, this, [this]() {
        closeCurrentTab();
    });

    connect(addressBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToAddressBar);

    // Tab events
    connect(tabs, &QTabWidget::currentChanged, this, [this](int) {
        syncAddressBarWithCurrentTab();
    });

    connect(tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTabByIndex);
}

QWebEngineView* MainWindow::createTab(const QUrl &url)
{
    auto *view = new QWebEngineView(tabs);

    int index = tabs->addTab(view, "New Tab");
    tabs->setCurrentIndex(index);

    connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) {
        int i = tabs->indexOf(view);
        if (i != -1) {
            tabs->setTabText(i, title.isEmpty() ? "New Tab" : title.left(40));
        }

        if (view == currentView()) {
            setWindowTitle((title.isEmpty() ? "BitBrowser" : title) + " - BitBrowser");
        }
    });

    connect(view, &QWebEngineView::urlChanged, this, [this, view](const QUrl &newUrl) {
        if (view == currentView()) {
            addressBar->setText(newUrl.toString());
        }
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
    auto *view = currentView();
    if (!view) return;

    const QUrl url = QUrl::fromUserInput(addressBar->text().trimmed());
    view->load(url);
}

void MainWindow::syncAddressBarWithCurrentTab()
{
    auto *view = currentView();
    if (!view) {
        addressBar->clear();
        setWindowTitle("BitBrowser");
        return;
    }

    addressBar->setText(view->url().toString());
}

void MainWindow::closeCurrentTab()
{
    int index = tabs->currentIndex();
    closeTabByIndex(index);
}

void MainWindow::closeTabByIndex(int index)
{
    if (index < 0) return;

    QWidget *w = tabs->widget(index);
    tabs->removeTab(index);
    delete w;

    if (tabs->count() == 0) {
        createTab();
    }
}
