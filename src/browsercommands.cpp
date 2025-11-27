#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

#include "browsercommands.h"
#include "bugmanager.h"
#include "operations.h"
#include "ImageViewWindow.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriTools.h"

namespace BrowserCommands {

Command::~Command() {}

QString Command::format(const QString& title) const
{
    return QString("<a href='cmd://%1'>%2</a>").arg(_cmd, title);
}

QString Command::format(const QString& arg1, const QString& title) const
{
    return QString("<a href='cmd://%1?%2=%3'>%4</a>").arg(_cmd, _arg1, arg1, title);
}

QString Command::format(const QString& arg1, const QString& arg2, const QString& title) const
{
    return QString("<a href='cmd://%1?%2=%3&%4=%5'>%6</a>").arg(_cmd, _arg1, arg1, _arg2, arg2, title);
}

QString Command::format(int arg1, const QString& title) const { return format(QString::number(arg1), title); }
QString Command::format(int arg1, int arg2, const QString& title) const { return format(QString::number(arg1), QString::number(arg2), title); }
int Command::arg1Int(const QUrl& url) const { return Ori::Tools::getParamInt(url, _arg1); }
int Command::arg2Int(const QUrl& url) const { return Ori::Tools::getParamInt(url, _arg2); }
QString Command::arg1Str(const QUrl& url) const { return Ori::Tools::getParamStr(url, _arg1); }
QString Command::arg2Str(const QUrl& url) const { return Ori::Tools::getParamStr(url, _arg2); }

//-----------------------------------------------------------------------------

class CommandShowRelated : public Command
{
public:
    CommandShowRelated(const QString& cmd, const QString& arg) : Command(cmd, arg) {}
    void exec(const QUrl& url) const override;
};

void CommandShowRelated::exec(const QUrl& url) const
{
    Operations::showIssue(arg1Int(url));
}

//-----------------------------------------------------------------------------

class CommandDelRelated : public Command
{
public:
    CommandDelRelated(const QString& cmd, const QString& arg1, const QString& arg2) : Command(cmd, arg1, arg2) {}
    void exec(const QUrl& url) const override;
};

void CommandDelRelated::exec(const QUrl& url) const
{
    Operations::deleteRelation(arg1Int(url), arg2Int(url));
}

//-----------------------------------------------------------------------------

class CommandShowImage : public Command
{
public:
    CommandShowImage(const QString& cmd, const QString& arg) : Command(cmd, arg) {}
    void exec(const QUrl& url) const override;
};

static QStringList supportedImgExts()
{
    static const QStringList suffixes({".png", ".jpg", ".jpeg"});
    return suffixes;
}

bool isSupportedImg(const QString& path)
{
    for (auto& suffix : supportedImgExts())
        if (path.endsWith(suffix, Qt::CaseInsensitive))
            return true;
    return false;
}

void CommandShowImage::exec(const QUrl& url) const
{
    auto fileName = arg1Str(url);
    QFileInfo file = BugManager::fileInDatabaseFiles(fileName);
    if (!file.exists())
    {
        bool fileNotFound = true;
        auto sourcePath = file.absoluteFilePath();
        // For old-style image tag [[Img:file]]
        // try to guess file ext if it's not provided
        for (auto& suffix : supportedImgExts())
        {
            if (!sourcePath.endsWith(suffix, Qt::CaseInsensitive))
            {
                file.setFile(sourcePath + suffix);
                if (file.exists())
                {
                    fileNotFound = false;
                    break;
                }
            }
        }
        if (fileNotFound)
            return Ori::Dlg::warning(qApp->tr("Image file not found:\n%1").arg(sourcePath));
    }
    ImageViewWindow::showImage(file, qApp->activeWindow());
}

//-----------------------------------------------------------------------------

class CommandGetFile : public Command
{
public:
    CommandGetFile(const QString& cmd, const QString& arg) : Command(cmd, arg) {}
    void exec(const QUrl& url) const override;
};

void CommandGetFile::exec(const QUrl& url) const
{
    auto fileName = arg1Str(url);
    QFileInfo file = BugManager::fileInDatabaseFiles(fileName);
    if (!file.exists())
        return Ori::Dlg::warning(qApp->tr("File not found:\n%1").arg(QDir::toNativeSeparators(file.filePath())));

    auto targetFile = QFileDialog::getSaveFileName(qApp->activeWindow(), qApp->tr("Export Attached File"));
    if (targetFile.isEmpty()) return;

    QFile source(file.filePath());
    if (!source.open(QIODevice::ReadOnly))
        return Ori::Dlg::error(qApp->tr("Unable to open source file '%1' for reading\n\n%2")
                               .arg(file.filePath(), source.errorString()));

    QFile target(targetFile);
    if (!target.open(QIODevice::WriteOnly))
        return Ori::Dlg::error(qApp->tr("Unable to open target file '%1' for writing\n\n%2")
                               .arg(targetFile, target.errorString()));

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

//-----------------------------------------------------------------------------

const Command& copySummary() { static Command c("copysummary");  return c; }
const Command& showText() { static Command c("showtext", "id");  return c; }
const Command& addComment() { static Command c("addcomment");  return c; }
const Command& makeRelation() { static Command c("makerelation");  return c; }
const Command& showRelated() { static CommandShowRelated c("showrelated", "id"); return c; }
const Command& delRelated() { static CommandDelRelated c("delrelated", "id1", "id2"); return c; }
const Command& showImage() { static CommandShowImage c("showimage", "file"); return c; }
const Command& getFile() { static CommandGetFile c("getfile", "file"); return c; }
const Command& showAllRelations() { static Command c("showallrels"); return c; }
const Command& showOpenedRelations() { static Command c("showopenrels"); return c; }

static const QString marker() { return QStringLiteral("cmd"); }

QString getCommand(const QUrl& url)
{
    return (url.scheme() == marker())? url.host(): QString();
}

QString getHint(const QUrl& url)
{
    QString cmd = getCommand(url);
    // TODO put commands to list and iterate over
    if (copySummary() == cmd) return qApp->tr("Copy number and summary to the clipboard");
    if (showText() == cmd) return qApp->tr("Show text changes");
    if (makeRelation() == cmd) return qApp->tr("Make new relation");
    if (showRelated() == cmd) return qApp->tr("Show issue #%1 in new page").arg(showRelated().arg1Str(url));
    if (delRelated() == cmd) return qApp->tr("Delete relation #%1").arg(delRelated().arg1Str(url));
    if (showImage() == cmd) return qApp->tr("Show image %1").arg(showImage().arg1Str(url));
    if (getFile() == cmd) return qApp->tr("Download file %1").arg(getFile().arg1Str(url));
    if (showAllRelations() == cmd) return qApp->tr("Show all relations");
    if (showOpenedRelations() == cmd) return qApp->tr("Show only opened relations");
    return QString();
}

bool isCommand(const QUrl& url)
{
    return url.scheme() == marker();
}

} // namespace BrowserCommands
