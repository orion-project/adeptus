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
    Command(const QString& cmd, const QString& arg): _cmd(cmd), _arg(arg) {}
    const QString cmd() const { return _cmd; }
    const QString arg() const { return _arg; }
    int argInt(const QUrl& url) const;
    QString argStr(const QUrl& url) const;
    QString format(const QString& title) const;
    QString format(const QString& arg, const QString& title) const;
    QString format(int arg, const QString& title) const;
    bool operator == (const QString& cmd) const { return _cmd == cmd; }
    virtual void exec(const QUrl&) const {}
private:
    QString _cmd, _arg;
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

QString getHint(const QString& cmd);

} // namespace BrowserCommands

#endif // BROWSERCOMMANDS_H
