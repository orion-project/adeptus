#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

#include "browsercommands.h"
#include "bugmanager.h"
#include "ImageViewWindow.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriTools.h"

namespace BrowserCommands {

QString Command::format(const QString& title) const
{
    return QString("<a href='cmd://%1'>%2</a>").arg(_cmd, title);
}

QString Command::format(const QString& arg, const QString& title) const
{
    return QString("<a href='cmd://%1?%2=%3'>%4</a>").arg(_cmd, _arg, arg, title);
}

QString Command::format(int arg, const QString& title) const
{
    return format(QString::number(arg), title);
}

int Command::argInt(const QUrl& url) const
{
    return Ori::Tools::getParamInt(url, _arg);
}

QString Command::argStr(const QUrl& url) const
{
    return Ori::Tools::getParamStr(url, _arg);
}

//-----------------------------------------------------------------------------

class CommandShowImage : public Command
{
public:
    CommandShowImage(const QString& cmd, const QString& arg) : Command(cmd, arg) {}

    void exec(const QUrl& url) const override
    {
        showImage(argStr(url));
    }

    void showImage(const QString& fileName) const
    {
        QFileInfo file = BugManager::fileInDatabaseFiles(fileName);
        if (!file.exists())
            return Ori::Dlg::warning(qApp->tr("File not found:\n%1")
                                     .arg(QDir::toNativeSeparators(file.filePath())));
        ImageViewWindow::showImage(file, qApp->activeWindow());
    }
};

//-----------------------------------------------------------------------------

class CommandGetFile : public Command
{
public:
    CommandGetFile(const QString& cmd, const QString& arg) : Command(cmd, arg) {}

    void exec(const QUrl& url) const override
    {
        processFileLink(argStr(url));
    }

    void processFileLink(const QString &fileName) const
    {
        QFileInfo file = BugManager::fileInDatabaseFiles(fileName);
        if (!file.exists())
            return Ori::Dlg::warning(qApp->tr("File not found:\n%1").arg(QDir::toNativeSeparators(file.filePath())));

        auto targetFile = QFileDialog::getSaveFileName(qApp->activeWindow(), qApp->tr("Export Attached File"));
        if (targetFile.isEmpty()) return;

        QFile source(file.filePath());
        if (!source.open(QIODevice::ReadOnly))
            return Ori::Dlg::error(qApp->tr("Unable to open source file '%1' for reading\n\n%2")
                                   .arg(file.filePath()).arg(source.errorString()));

        QFile target(targetFile);
        if (!target.open(QIODevice::WriteOnly))
            return Ori::Dlg::error(qApp->tr("Unable to open target file '%1' for writing\n\n%2")
                                   .arg(targetFile).arg(target.errorString()));

        QDataStream sourceData(&source);
        QDataStream targetData(&target);

        char buf[4096];
        int bytesRead;
        while ((bytesRead = sourceData.readRawData(&buf[0], 4096)) != 0)
        {
            if (bytesRead < 0)
                return Ori::Dlg::error(qApp->tr("Error reading from source file '%1'\n\n%2")
                   .arg(source.fileName()).append(source.errorString()));

            if (targetData.writeRawData(buf, bytesRead) < 0)
                return Ori::Dlg::error(qApp->tr("Error writing to target file '%1'\n\n%2")
                   .arg(target.fileName()).append(target.errorString()));
        }
    }
};

//-----------------------------------------------------------------------------

const Command& copySummary() { static Command c("copysummary");  return c; }
const Command& showText() { static Command c("showtext", "id");  return c; }
const Command& addComment() { static Command c("addcomment");  return c; }
const Command& makeRelation() { static Command c("makerelation");  return c; }
const Command& showRelated() { static Command c("showrelated", "id"); return c; }
const Command& delRelated() { static Command c("delrelated", "id"); return c; }
const Command& showImage() { static CommandShowImage c("showimage", "file"); return c; }
const Command& getFile() { static CommandGetFile c("getfile", "file"); return c; }
const Command& showAllRelations() { static Command c("showallrels"); return c; }
const Command& showOpenedRelations() { static Command c("showopenrels"); return c; }


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
