#include "browsercommands.h"
#include "helpers/OriTools.h"

#include <QApplication>

namespace BrowserCommands {

QString CommandDef::format(const QString& title) const
{
    return QString("<a href='cmd://%1'>%2</a>").arg(_cmd, title);
}

QString CommandDef::format(const QString& arg, const QString& title) const
{
    return QString("<a href='cmd://%1?%2=%3'>%4</a>").arg(_cmd, _arg, arg, title);
}

QString CommandDef::format(int arg, const QString& title) const
{
    return format(QString::number(arg), title);
}

int CommandDef::argInt(const QUrl& url) const
{
    return Ori::Tools::getParamInt(url, _arg);
}

QString CommandDef::argStr(const QUrl& url) const
{
    return Ori::Tools::getParamStr(url, _arg);
}

const CommandDef& copySummary() { static CommandDef c("copysummary");  return c; }
const CommandDef& showText() { static CommandDef c("showtext", "id");  return c; }
const CommandDef& addComment() { static CommandDef c("addcomment");  return c; }
const CommandDef& makeRelation() { static CommandDef c("makerelation");  return c; }
const CommandDef& showRelated() { static CommandDef c("showrelated", "id"); return c; }
const CommandDef& delRelated() { static CommandDef c("delrelated", "id"); return c; }
const CommandDef& showImage() { static CommandDef c("showimage", "file"); return c; }
const CommandDef& getFile() { static CommandDef c("getfile", "file"); return c; }
const CommandDef& showAllRelations() { static CommandDef c("showallrels"); return c; }
const CommandDef& showOpenedRelations() { static CommandDef c("showopenrels"); return c; }


QString getHint(const QString& cmd)
{
    if (copySummary() == cmd) return qApp->tr("Copy number and summary to the clipboard");
    if (showText() == cmd) return qApp->tr("Show text changes");
    if (delRelated() == cmd) return qApp->tr("Delete relation");
    if (showRelated() == cmd) return qApp->tr("Show issue in new page");
    if (makeRelation() == cmd) return qApp->tr("Make new relation");
    if (showAllRelations() == cmd) return qApp->tr("Show all relations");
    if (showOpenedRelations() == cmd) return qApp->tr("Show only opened relations");
    return QString();
}

} // namespace BrowserCommands
