#ifndef BUGHISTORY_H
#define BUGHISTORY_H

#include <QWidget>
#include <QMap>

#include "bugtypes.h"

QT_BEGIN_NAMESPACE
class QTextBrowser;
class QSqlRecord;
QT_END_NAMESPACE

class IssueTextView;

class BugHistory : public QWidget
{
    Q_OBJECT

public:
    explicit BugHistory(int id, QWidget *parent = 0);

    void populate();
    int id() const { return _id; }
    int status() const { return _status; }
    const QString& title() const { return _summary; }
    const QList<int>& relatedIds() const { return _relatedIds; }
    void setFocus();

private:
    IssueTextView* contentView;
    QString _summary;
    QMap<int, QPair<QString, QString> > _changedTexts;
    int _id, _status, _changedTextIndex;
    QList<int> _relatedIds;
    bool _showOnlyOpenedRelations = false;

    QString formatSectionTitle(const QString& title);
    QString formatSummary(const BugInfo &bug);
    QString formatRelations();
    QString formatRelationsCount(int countOpened);
    QString formatHistory();
    QString formatChangedParams(const QList<BugHistoryItem::ChangedParam>& params);
    QString formatChangedParam(const BugHistoryItem::ChangedParam& param);
    QString finishWithError(const QString& content, const QString& error);

    void showChangedText(int id);
    void scrollToEnd();
    void setShowOnlyOpenedRelations(bool on);

    QString headerClass() const;

    void processCommand(const QString& cmd, const QUrl& url);

private slots:
    void commentAdded(int bugId);
};

#endif // BUGHISTORY_H
