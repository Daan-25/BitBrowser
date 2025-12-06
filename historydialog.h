#pragma once

#include <QDialog>
#include <QVector>
#include <QUrl>
#include <QDateTime>

class QListWidget;
class QLineEdit;

struct HistoryEntry
{
    QUrl url;
    QString title;
    QDateTime visitedAt;
};

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(const QVector<HistoryEntry> &entries, QWidget *parent = nullptr);

signals:
    void openUrlRequested(const QUrl &url);
    void clearRequested();

private:
    void rebuildList(const QString &filterText = QString());

    QVector<HistoryEntry> m_entries;

    QLineEdit *searchEdit = nullptr;
    QListWidget *listWidget = nullptr;
};
