#include "bookmarksdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

BookmarksDialog::BookmarksDialog(const QVector<BookmarkEntry> &entries, QWidget *parent)
    : QDialog(parent), m_entries(entries)
{
    setWindowTitle("Bookmarks");
    resize(800, 500);

    auto *root = new QVBoxLayout(this);

    auto *topRow = new QHBoxLayout();
    auto *searchLabel = new QLabel("Search:", this);
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Filter by title or URL...");

    auto *removeBtn = new QPushButton("Remove selected", this);
    auto *clearBtn = new QPushButton("Clear all", this);

    topRow->addWidget(searchLabel);
    topRow->addWidget(searchEdit, 1);
    topRow->addWidget(removeBtn);
    topRow->addWidget(clearBtn);

    listWidget = new QListWidget(this);

    root->addLayout(topRow);
    root->addWidget(listWidget);

    rebuildList();

    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        rebuildList(text);
    });

    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        auto *item = listWidget->currentItem();
        if (!item) return;

        const QUrl url = item->data(Qt::UserRole).toUrl();
        if (url.isValid())
            emit removeUrlRequested(url);
    });

    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        emit clearRequested();
        m_entries.clear();
        rebuildList();
    });

    connect(listWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        const QUrl url = item->data(Qt::UserRole).toUrl();
        if (url.isValid())
            emit openUrlRequested(url);
    });
}

void BookmarksDialog::rebuildList(const QString &filterText)
{
    listWidget->clear();

    const QString filter = filterText.trimmed();

    for (const auto &e : m_entries)
    {
        const QString title = e.title.isEmpty() ? e.url.toString() : e.title;
        const QString line = title;

        if (!filter.isEmpty())
        {
            const QString hay = (title + " " + e.url.toString());
            if (!hay.contains(filter, Qt::CaseInsensitive))
                continue;
        }

        auto *item = new QListWidgetItem(line, listWidget);
        item->setToolTip(e.url.toString());
        item->setData(Qt::UserRole, e.url);
    }

    if (listWidget->count() == 0)
    {
        auto *empty = new QListWidgetItem("No bookmarks yet.", listWidget);
        empty->setFlags(Qt::NoItemFlags);
    }
}
