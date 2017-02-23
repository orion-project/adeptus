#ifndef BROWSERCOMMANDS_H
#define BROWSERCOMMANDS_H

#include <QString>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

namespace BrowserCommands {

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

const CommandDef& copySummary();
const CommandDef& showText();
const CommandDef& addComment();
const CommandDef& makeRelation();
const CommandDef& showRelated();
const CommandDef& delRelated();
const CommandDef& showImage();
const CommandDef& getFile();
const CommandDef& showAllRelations();
const CommandDef& showOpenedRelations();

QString getHint(const QString& cmd);

} // namespace BrowserCommands

#endif // BROWSERCOMMANDS_H
