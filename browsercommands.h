#ifndef BROWSERCOMMANDS_H
#define BROWSERCOMMANDS_H

#include <QString>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

namespace BrowserCommands {

class Command
{
public:
    Command(const QString& cmd): _cmd(cmd) {}
    Command(const QString& cmd, const QString& arg1): _cmd(cmd), _arg1(arg1) {}
    Command(const QString& cmd, const QString& arg1, const QString& arg2): _cmd(cmd), _arg1(arg1), _arg2(arg2) {}
    const QString cmd() const { return _cmd; }
    const QString arg1() const { return _arg1; }
    const QString arg2() const { return _arg2; }
    int arg1Int(const QUrl& url) const;
    int arg2Int(const QUrl& url) const;
    QString arg1Str(const QUrl& url) const;
    QString arg2Str(const QUrl& url) const;
    QString format(const QString& title) const;
    QString format(const QString& arg1, const QString& title) const;
    QString format(const QString& arg1, const QString& arg2, const QString& title) const;
    QString format(int arg1, const QString& title) const;
    QString format(int arg1, int arg2, const QString& title) const;
    bool operator == (const QString& cmd) const { return _cmd == cmd; }
    virtual void exec(const QUrl&) const {}
private:
    QString _cmd, _arg1, _arg2;
};

const Command& copySummary();
const Command& showText();
const Command& addComment();
const Command& makeRelation();
const Command& showRelated();
const Command& delRelated();
const Command& showImage();
const Command& getFile();
const Command& showAllRelations();
const Command& showOpenedRelations();

void processCommand(const QUrl& url);
QString getHint(const QUrl &url);

} // namespace BrowserCommands

#endif // BROWSERCOMMANDS_H
