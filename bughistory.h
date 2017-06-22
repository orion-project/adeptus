#ifndef BUGHISTORY_H
#define BUGHISTORY_H

#include <QWidget>
#include <QMap>

#include "bugtypes.h"

QT_BEGIN_NAMESPACE
class QTextBrowser;
class QSqlRecord;
QT_END_NAMESPACE

class BugHistory : public QWidget
{
    Q_OBJECT

public:
    explicit BugHistory(int id, QWidget *parent = 0);
    ~BugHistory();

    void populate();
    int id() const { return _id; }
    int status() const { return _status; }
    const QString& title() const { return _summary; }
    const QList<int>& relatedIds() const { return _relatedIds; }
    void setFocus();

signals:
    void operationRequest(int operation, int id);

private:
    QTextBrowser* contentView;
    QString _summary;
    QMap<int, QPair<QString, QString> > _changedTexts;
    int _id, _status, _changedTextIndex;
    QList<int> _relatedIds;
    //BugProvider* _bugProvider;
    bool _showOnlyOpenedRelations = false;

    QString formatSectionTitle(const QString& title);
    QString formatSummary(const BugInfo &bug);
    QString formatRelations();
    QString formatRelationsCount(int countOpened);
    QString formatHistory();
    QString formatChangedParams(const QList<BugHistoryItem::ChangedParam>& params);
    QString formatChangedParam(const BugHistoryItem::ChangedParam& param);
    QString finishWithError(const QString& content, const QString& error);
    QString processWikiTags(const QString& s);

    void showChangedText(int id);
    void scrollToEnd();
    void setShowOnlyOpenedRelations(bool on);

    QString headerClass() const;

private slots:
    void linkClicked(const class QUrl&);
    void linkHovered(const class QUrl&);
    void commentAdded(int bugId);
};

#endif // BUGHISTORY_H
