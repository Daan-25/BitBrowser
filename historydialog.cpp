#include "historydialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

HistoryDialog::HistoryDialog(const QVector<HistoryEntry> &entries, QWidget *parent)
    : QDialog(parent), m_entries(entries)
{
    setWindowTitle("History");
    resize(800, 500);

    auto *root = new QVBoxLayout(this);

    // Top bar: search + clear button
    auto *topRow = new QHBoxLayout();
    auto *searchLabel = new QLabel("Search:", this);
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Filter by title or URL...");

    auto *clearBtn = new QPushButton("Clear history", this);

    topRow->addWidget(searchLabel);
    topRow->addWidget(searchEdit, 1);
    topRow->addWidget(clearBtn);

    listWidget = new QListWidget(this);

    root->addLayout(topRow);
    root->addWidget(listWidget);

    rebuildList();

    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        rebuildList(text);
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

void HistoryDialog::rebuildList(const QString &filterText)
{
    listWidget->clear();

    // Newest first
    QVector<HistoryEntry> sorted = m_entries;
    std::sort(sorted.begin(), sorted.end(), [](const HistoryEntry &a, const HistoryEntry &b) {
        return a.visitedAt > b.visitedAt;
    });

    const QString filter = filterText.trimmed();

    for (const auto &e : sorted)
    {
        const QString title = e.title.isEmpty() ? e.url.toString() : e.title;
        const QString line = QString("%1  —  %2")
                                 .arg(e.visitedAt.toString("yyyy-MM-dd HH:mm"))
                                 .arg(title);

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
        auto *empty = new QListWidgetItem("No history entries.", listWidget);
        empty->setFlags(Qt::NoItemFlags);
    }
}
