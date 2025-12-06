#pragma once

#include <QDialog>
#include <QVector>
#include <QUrl>

class QListWidget;
class QLineEdit;

struct BookmarkEntry
{
    QUrl url;
    QString title;
};

class BookmarksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarksDialog(const QVector<BookmarkEntry> &entries, QWidget *parent = nullptr);

signals:
    void openUrlRequested(const QUrl &url);
    void removeUrlRequested(const QUrl &url);
    void clearRequested();

private:
    void rebuildList(const QString &filterText = QString());

    QVector<BookmarkEntry> m_entries;

    QLineEdit *searchEdit = nullptr;
    QListWidget *listWidget = nullptr;
};
