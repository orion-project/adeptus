#ifndef BUGHISTORY_H
#define BUGHISTORY_H

#include <QWidget>
#include <QMap>

#include "bugtypes.h"

QT_BEGIN_NAMESPACE
class QTextBrowser;
class QSqlRecord;
QT_END_NAMESPACE

namespace Commands {

class CommandDef
{
public:
    CommandDef(const QString& cmd): _cmd(cmd) {}
    CommandDef(const QString& cmd, const QString& arg): _cmd(cmd), _arg(arg) {}
    const QString cmd() const { return _cmd; }
    const QString arg() const { return _arg; }
    int argInt(const QUrl& url) const;
    QString argStr(const QUrl& url) const;
    QString format(const QString& title) const;
    QString format(const QString& arg, const QString& title) const;
    QString format(int arg, const QString& title) const;
    bool operator == (const QString& cmd) const { return _cmd == cmd; }
private:
    QString _cmd, _arg;
};

} // namespace Commands

class BugHistory : public QWidget
{
    Q_OBJECT

public:
    explicit BugHistory(int id, QWidget *parent = 0);
    ~BugHistory();

    static void showDialog(int id, QWidget *parent = 0);

    void populate();
    int id() const { return _id; }
    int status() const { return _status; }
    const QString& title() const { return _title; }
    const QList<int>& relatedIds() const { return _relatedIds; }
    void setFocus();

signals:
    void operationRequest(int operation, int id);

private:
    QTextBrowser* contentView;
    QString _title, _summary;
    QMap<int, QPair<QString, QString> > _changedTexts;
    int _id, _status, _changedTextIndex;
    QList<int> _relatedIds;
    BugProvider* _bugProvider;
    bool _showOnlyOpenedRelations = false;

    QString formatSectionTitle(const QString& title);
    QString formatSummary(const QSqlRecord& record);
    QString formatRelations();
    QString formatRelationsCount(int countOpened);
    QString formatHistory();
    QString formatChangedParams(const QList<BugHistoryItem::ChangedParam>& params);
    QString formatChangedParam(const BugHistoryItem::ChangedParam& param);
    QString finishWithError(const QString& content, const QString& error);
    QString processWikiTags(const QString& s);
    QString processWikiTag_Bold(const QString& s, bool& ok);
    QString processWikiTag_Italic(const QString& s, bool& ok);
    QString processWikiTag_Image(const QString& s, bool& ok);
    QString processWikiTag_File(const QString& s, bool& ok);
    QString processWikiTag_Bug(const QString& s, bool& ok);
    QString processWikiTag_Resource(const QString& s, const QString& tag, const Commands::CommandDef& cmd, bool& ok);

    void showChangedText(int id);
    void deleteRelation(int id);
    void showImage(const QString &fileName);
    void processFileLink(const QString &fileName);
    void scrollToEnd();
    void setShowOnlyOpenedRelations(bool on);

    QString headerClass() const;

private slots:
    void linkClicked(const class QUrl&);
    void linkHovered(const class QUrl&);
    void commentAdded(int bugId);
};

#endif // BUGHISTORY_H
