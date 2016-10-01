#include <QtSql>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QScreen>
#include <QScrollArea>
#include <QTabWidget>
#include <QTextBrowser>
#include <QToolTip>

#include "appearance.h"
#include "bughistory.h"
#include "bugmanager.h"
#include "bugsolver.h"
#include "bugeditor.h"
#include "bugoperations.h"
#include "ImageViewWindow.h"
#include "SqlBugProvider.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriTools.h"
#include "helpers/OriWidgets.h"

namespace Commands {

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
}

//-------------------------------------------------------------------------------------------------------

void BugHistory::showDialog(int id, QWidget *parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(qApp->tr("Issue #%1 History").arg(id));
    dlg.setSizeGripEnabled(true);
    dlg.setLayout(new QVBoxLayout);
    dlg.layout()->setMargin(0);
    dlg.layout()->addWidget(new BugHistory(id));
    dlg.resize(800, 480);
    dlg.exec();
}

BugHistory::BugHistory(int id, QWidget *parent) : QWidget(parent), 
    _id(id), _status(-1), _changedTextIndex(0)
{
    _bugProvider = new SqlBugProvider;

    contentView = new QTextBrowser;
    contentView->setReadOnly(true);
    contentView->setOpenLinks(false);
    contentView->setStyleSheet(QString("QTextBrowser{background-color: %1; border-style: none;}")
                               .arg(palette().color(QPalette::Window).name()));
    connect(contentView, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    connect(contentView, SIGNAL(highlighted(QUrl)), this, SLOT(linkHovered(QUrl)));

    Ori::Gui::adjustFont(contentView);

    contentView->document()->setDefaultStyleSheet(
                QString(".header { background-color: %2; }"
                        ".header_solved { background-color: %4; }"
                        ".header_closed { background-color: %5; }"
                        ".props { background-color: %3; }"
                        ".summary { background-color: %1; }"
                        ".extra { background-color: %1; }"
                        ".solved_ref { background-color: %4; }"
                        ".closed_ref { background-color: %5; }")
                .arg(palette().color(QPalette::Base).name())
                .arg(palette().color(QPalette::Midlight).name())
                .arg(palette().color(QPalette::AlternateBase).name())
                .arg(ColorProvider::solvedColor().name())
                .arg(ColorProvider::closedColor().name())
        );

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setMargin(0);
    layoutMain->addWidget(contentView);
    setLayout(layoutMain);

    connect(BugOperations::instance(), SIGNAL(bugCommentAdded(int)), this, SLOT(commentAdded(int)));
}

BugHistory::~BugHistory()
{
    delete _bugProvider;
}

void BugHistory::populate()
{
    QString content;
    QSqlRecord record = BugManager::bug(_id, content);
    if (content.isEmpty())
    {
        _title = record.field(COL_SUMMARY).value().toString();
        _status = record.field(COL_STATUS).value().toInt();

        content = formatSummary(record) % formatRelations() % formatHistory() %
            "<p>" % Commands::addComment().format(tr("Append comment")) %
             "&nbsp;&nbsp;&nbsp;" %
             Commands::makeRelation().format(tr("Make relation"));
    }
    else
    {
        _title.clear();
        _status = -1;
    }
    contentView->setHtml(content);
}

inline bool ok(const QString& result)
{
    return result.isEmpty();
}

QString sanitizeHtml(const QString& s)
{
    return QString(s).replace("<", "&lt;").replace(">", "&gt;").replace("\n\n", "<p>").replace("\n", "<br>");
}

QString formatError(const QString& s)
{
    return "<span style='color:red'>" + sanitizeHtml(s) + "</span>";
}

QString formatMoment(const QDateTime& moment)
{
    return QString::fromLatin1("<nobr>%1 <span style='color:#555'>%2</span></nobr>")
        .arg(moment.date().toString(Qt::SystemLocaleShortDate))
        .arg(moment.time().toString(Qt::SystemLocaleShortDate));
}

QString BugHistory::formatSummary(const QSqlRecord& record)
{
    _summary = record.field(COL_SUMMARY).value().toString();

    QString content("<table border=1 width=100% cellspacing=0 cellpadding=5>");
    content += QString("<tr class='%7'><td>"
                            "<table width=100%><tr>"
                                "<td rowspan=2 valign=middle style='font-size:11pt'><b>%1</b> %2</td>"
                                "<td align=right><nobr><span style='color:gray'>%3:</span> %4</nobr></td></tr>"
                                "<tr><td align=right><nobr><span style='color:gray'>%5:</span> %6</nobr></td></tr>"
                            "</table>"
                       "</td></tr>")
            .arg(Commands::copySummary().format(QString("#%1").arg(_id))).arg(sanitizeHtml(_summary))
            .arg(tr("Created")).arg(formatMoment(record.field(COL_CREATED).value().toDateTime()))
            .arg(tr("Updated")).arg(formatMoment(record.field(COL_UPDATED).value().toDateTime()))
            .arg(headerClass());

    content += "<tr class='props'><td>";
    foreach (int dictId, BugManager::dictionaryIds())
    {
        int valueId = record.value(dictId).toInt();
        QString value = sanitizeHtml(BugManager::dictionaryCash(dictId)->value(valueId));
        content += QString("%1: <b>%2</b>. ").arg(BugManager::columnTitle(dictId)).arg(value);
    }
    content += "</td></tr>";

    QString extra = record.field(COL_EXTRA).value().toString().trimmed();
    if (!extra.isEmpty())
        content += QString("<tr class='extra'><td>%1</td></tr>").arg(processWikiTags(sanitizeHtml(extra)));

    return content + "</table>";
}

QString BugHistory::headerClass() const
{
    switch (_status)
    {
        case STATUS_CLOSED: return QStringLiteral("header_closed");
        case STATUS_SOLVED: return QStringLiteral("header_solved");
        default: return QStringLiteral("header");
    }
}

QString BugHistory::formatSectionTitle(const QString& title)
{
    return "<p><b>" % title % "</b>";
}

QString BugHistory::finishWithError(const QString& content, const QString& error)
{
    return content % "<p>" % formatError(error);
}

QString BugHistory::formatRelations()
{
    QString sectionTitle = formatSectionTitle(tr("Related Issues"));

    IntListResult res = _bugProvider->getRelations(_id);
    if (!res.ok()) return finishWithError(sectionTitle, res.error());

    _relatedIds = res.result();
    if (_relatedIds.isEmpty()) return QString();

    QString content = QString("<table border=1 width=100% cellspacing=0 cellpadding=5>");
    int countOpened = 0;
    for (int relatedId : _relatedIds)
    {
        QString moment, command;
        QString row_class("extra");
        QString title = QString("#%1: ").arg(relatedId);
        BugResult res = _bugProvider->getBug(relatedId);
        if (res.ok())
        {
            BugInfo bug = res.result();
            QString status = DictManager::status(bug.status);
            if (!_bugProvider->isBugOpened(bug.status))
            {
                if (_showOnlyOpenedRelations)
                    continue;

                row_class = bug.status == STATUS_CLOSED? "closed_ref": "solved_ref";
                status += ":" + DictManager::solution(bug.solution);
            }
            else countOpened++;

            title += QString("(%1) ").arg(status) %
                Commands::showRelated().format(relatedId, sanitizeHtml(bug.summary));
            moment = formatMoment(bug.created);
            command = Commands::delRelated().format(relatedId, "<img src=':/tools/delete'>");
        }
        else title += formatError(res.error());

        content += QString("<tr class='%1'><td>"
                               "<table width=100%><tr>"
                                   "<td>%2</td>"
                                   "<td align=right>%3</td>"
                                   "<td align=right width=20>%4</td>"
                                "</tr></table>"
                           "</td></tr>")
                .arg(row_class).arg(title).arg(moment).arg(command);
    }
    content += "</table>";

    QString title = QString::fromLatin1("<p><table width=100%><tr>"
                                   "<td valign=bottom>%1&nbsp;(%2)</td>"
                                   "<td align=right width=32>%3&nbsp;</td>"
                                "</tr></table>")
        .arg(sectionTitle)
        .arg(formatRelationsCount(countOpened))
        .arg(Commands::makeRelation().format("<img src=':/tools/plus'>"));


    return title + content;
}

QString BugHistory::formatRelationsCount(int countOpened)
{
    auto countAll = QString::number(_relatedIds.size());
    auto countOpen = QString::number(countOpened);
    return tr("%1 %2, %3 %4")
        .arg(Commands::showAllRelations().format(tr("all:", "Show all relations command")))
        .arg(_showOnlyOpenedRelations? countAll: ("<b>" % countAll % "</b>"))
        .arg(Commands::showOpenedRelations().format(tr("opened:", "Show opened relations command")))
        .arg(_showOnlyOpenedRelations? ("<b>"% countOpen % "</b>"): countOpen);
}

QString BugHistory::formatHistory()
{
    QString content = "<p><b>" % tr("Issue History:") % "</b>";

    BugHistoryResult res = _bugProvider->getHistory(_id);
    if (!res.ok()) return content % "<p>" % formatError(res.error());

    BugHistoryItems history = res.result();
    if (history.isEmpty()) return QString();

    for (int i = 0; i < history.size(); i++)
    {
        const BugHistoryItem& item = history.at(i);

        content += QString("<table border=1 width=100% cellspacing=0 cellpadding=5>"
                                "<tr class='header'><td>"
                                    "<table width=100%><tr>"
                                        "<td><b>%1</b></td>"
                                        "<td align=right><nobr>%2</nobr></td>"
                                    "</tr></table>"
                                "</td></tr>")
                .arg(item.number)
                .arg(formatMoment(item.moment));

        QString changedParams = formatChangedParams(item.changedParams);
        if (!changedParams.isEmpty())
            content += QString("<tr class='props'><td>%1</td></tr>").arg(changedParams);

        if (!item.comment.isEmpty())
            content += QString("<tr class='extra'><td>%1</td></tr>")
                    .arg(processWikiTags(sanitizeHtml(item.comment)));

        content += "</table>";

        if (i < history.size()-1) content += "<br>";
    }
    return content;
}

QString BugHistory::formatChangedParams(const QList<BugHistoryItem::ChangedParam>& params)
{
    QStringList strs;
    for (const BugHistoryItem::ChangedParam& p : params)
        strs.append(formatChangedParam(p));
    return strs.join(". ");
}

QString BugHistory::formatChangedParam(const BugHistoryItem::ChangedParam& param)
{
    QString paramName = _bugProvider->bugParamName(param.paramId);
    QVariant oldValue = param.oldValue;
    QVariant newValue = param.newValue;
    if (param.paramId == COL_SUMMARY || param.paramId == COL_EXTRA)
    {
        _changedTexts[++_changedTextIndex] = QPair<QString, QString>(
                    oldValue.toString(), newValue.toString());
        return QString("%1: %2").arg(tr("Changed")).arg(
            Commands::showText().format(_changedTextIndex, paramName));
    }
    QMap<int, QString> *dict = BugManager::dictionaryCash(param.paramId);
    if (dict)
    {
        if (dict->contains(oldValue.toInt()))
            oldValue = dict->value(oldValue.toInt());
        if (dict->contains(newValue.toInt()))
            newValue = dict->value(newValue.toInt());
    }
    return QString("%1: <b>%2 -> %3</b>")
            .arg(sanitizeHtml(paramName))
            .arg(sanitizeHtml(oldValue.toString()))
            .arg(sanitizeHtml(newValue.toString()));
}

void BugHistory::linkClicked(const QUrl& url)
{
    if (url.scheme() == "cmd")
    {
        QString cmd = url.host();
        if (Commands::copySummary() == cmd)
            QApplication::clipboard()->setText(QString("#%1: %2").arg(_id).arg(_summary));

        else if (Commands::addComment() == cmd)
           emit operationRequest(BugManager::Operation_Comment, 0);

        else if (Commands::makeRelation() == cmd)
            emit operationRequest(BugManager::Operation_MakeRelation, 0);

        else if (Commands::showText() == cmd)
            showChangedText(Commands::showText().argInt(url));

        else if (Commands::delRelated() == cmd)
            deleteRelation(Commands::delRelated().argInt(url));

        else if (Commands::showRelated() == cmd)
            emit operationRequest(BugManager::Operation_Show, Commands::showRelated().argInt(url));

        else if (Commands::showImage() == cmd)
            showImage(Commands::showImage().argStr(url));

        else if (Commands::getFile() == cmd)
            processFileLink(Commands::getFile().argStr(url));

        else if (Commands::showAllRelations() == cmd)
            setShowOnlyOpenedRelations(false);

        else if (Commands::showOpenedRelations() == cmd)
            setShowOnlyOpenedRelations(true);
    }
}

void BugHistory::linkHovered(const class QUrl& url)
{
    QString tooltip;
    if (url.scheme() == "cmd")
    {
        QString cmd = url.host();
        if (Commands::copySummary() == cmd) tooltip = tr("Copy number and summary to the clipboard");
        else if (Commands::showText() == cmd) tooltip = tr("Show text changes");
        else if (Commands::delRelated() == cmd) tooltip = tr("Delete relation");
        else if (Commands::showRelated() == cmd) tooltip = tr("Show issue in new page");
        else if (Commands::makeRelation() == cmd) tooltip = tr("Make new relation");
        else if (Commands::showAllRelations() == cmd) tooltip = tr("Show all relations");
        else if (Commands::showOpenedRelations() == cmd) tooltip = tr("Show only opened relations");
    }
    if (!tooltip.isEmpty())
        QToolTip::showText(QCursor::pos(), tooltip);
    else
        QToolTip::hideText();
}

void BugHistory::showChangedText(int id)
{
    if (!_changedTexts.contains(id)) return;

    QPlainTextEdit *oldValue = new QPlainTextEdit;
    oldValue->setStyleSheet("border-style: none");
    oldValue->setPlainText(_changedTexts[id].first);
    oldValue->setFont(contentView->font());
    oldValue->setReadOnly(true);

    QPlainTextEdit *newValue = new QPlainTextEdit;
    newValue->setStyleSheet("border-style: none");
    newValue->setPlainText(_changedTexts[id].second);
    newValue->setFont(contentView->font());
    newValue->setReadOnly(true);

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(oldValue, tr("Old value"));
    tabs->addTab(newValue, tr("New value"));

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Changed text"));
    dlg.setSizeGripEnabled(true);
    dlg.setLayout(new QVBoxLayout);
    dlg.layout()->addWidget(tabs);
    dlg.resize(600, 300);
    dlg.exec();
}

void BugHistory::deleteRelation(int id)
{
    if (Ori::Dlg::yes(tr("Delete relation [#%1 - #%2]?").arg(_id).arg(id)))
    {
        QString res = BugManager::deleteRelation(_id, id);
        if (!res.isEmpty())
        {
            Ori::Dlg::error(res);
            return;
        }
        populate();
        emit operationRequest(BugManager::Operation_Update, id);
    }
}

QString BugHistory::processWikiTags(const QString& s)
{
    bool ok;
    QString res = s;
    do { res = processWikiTag_Bold(res, ok); } while (ok);
    do { res = processWikiTag_Italic(res, ok); } while (ok);
    do { res = processWikiTag_Image(res, ok); } while (ok);
    do { res = processWikiTag_File(res, ok); } while (ok);
    do { res = processWikiTag_Bug(res, ok); } while (ok);
    return res;
}

QString BugHistory::processWikiTag_Bold(const QString& s, bool& ok)
{
    ok = false;
    static QLatin1String tag("'''");

    int startPos = s.indexOf(tag, 0);
    if (startPos < 0) return s;

    int endPos = s.indexOf(tag, startPos+3);
    if (endPos < 0) return s;

    QStringRef strBegin(&s, 0, startPos);
    QStringRef strBolded(&s, startPos+3, endPos-(startPos+3));
    QStringRef strEnd(&s, endPos+3, s.length() - (endPos+3));
    ok = true;
    return strBegin % QStringLiteral("<b>") % strBolded % QStringLiteral("</b>") % strEnd;
}

QString BugHistory::processWikiTag_Italic(const QString& s, bool& ok)
{
    ok = false;
    static QLatin1String tag("''");

    int startPos = s.indexOf(tag, 0);
    if (startPos < 0) return s;

    int endPos = s.indexOf(tag, startPos+2);
    if (endPos < 0) return s;

    QStringRef strBegin(&s, 0, startPos);
    QStringRef strItalized(&s, startPos+2, endPos-(startPos+2));
    QStringRef strEnd(&s, endPos+2, s.length() - (endPos+2));
    ok = true;
    return strBegin % QStringLiteral("<i>") % strItalized % QStringLiteral("</i>") % strEnd;
}

QString BugHistory::processWikiTag_Image(const QString& s, bool& ok)
{
    return processWikiTag_Resource(s, QStringLiteral("Image"), Commands::showImage(), ok);
}

QString BugHistory::processWikiTag_File(const QString& s, bool& ok)
{
    return processWikiTag_Resource(s, QStringLiteral("File"), Commands::getFile(), ok);
}

QString BugHistory::processWikiTag_Bug(const QString& s, bool& ok)
{
    return processWikiTag_Resource(s, QStringLiteral("Bug"), Commands::showRelated(), ok);
}

QString BugHistory::processWikiTag_Resource(const QString& s, const QString& tag, const Commands::CommandDef &cmd, bool& ok)
{
    ok = false;
    static QLatin1String tagEnd("]]");
    static QLatin1String tagStart("[[");

    int startPos = s.indexOf(tagStart % tag % ':', 0, Qt::CaseInsensitive);
    if (startPos < 0) return s;

    int resourceStartPos = startPos + tag.size() + 3;
    int endPos = s.indexOf(tagEnd, resourceStartPos+1);
    if (endPos < 0) return s;

    QStringRef strBegin(&s, 0, startPos);
    QStringRef strMid(&s, resourceStartPos, endPos - resourceStartPos);
    QStringRef strEnd(&s, endPos+2, s.length() - (endPos+2));

    if (strMid.contains(tagStart)) return s;

    ok = true;
    QString resource = strMid.trimmed().toString();
    return strBegin % cmd.format(resource, resource) % strEnd;
}

void BugHistory::showImage(const QString& fileName)
{
    QFileInfo file = BugManager::fileInDatabaseFiles(fileName);
    if (!file.exists())
        return Ori::Dlg::warning(tr("File not found:\n%1").arg(QDir::toNativeSeparators(file.filePath())));

    auto label = new QLabel(this);
    label->setPixmap(QPixmap(file.filePath()));
    label->setToolTip(QDir::toNativeSeparators(file.filePath()));

    ImageViewWindow::showImage(file, this);
}

void BugHistory::processFileLink(const QString &fileName)
{
    QFileInfo file = BugManager::fileInDatabaseFiles(fileName);
    if (!file.exists())
        return Ori::Dlg::warning(tr("File not found:\n%1").arg(QDir::toNativeSeparators(file.filePath())));

    auto targetFile = QFileDialog::getSaveFileName(this, tr("Export Attached File"));
    if (!targetFile.isEmpty())
    {

        QFile source(file.filePath());
        if (!source.open(QIODevice::ReadOnly))
            return Ori::Dlg::error(tr("Unable to open source file '%1' for reading\n\n%2")
                                   .arg(file.filePath()).arg(source.errorString()));

        QFile target(targetFile);
        if (!target.open(QIODevice::WriteOnly))
            return Ori::Dlg::error(tr("Unable to open target file '%1' for writing\n\n%2")
                                   .arg(targetFile).arg(target.errorString()));

        QDataStream sourceData(&source);
        QDataStream targetData(&target);

        char buf[4096];
        int bytesRead;
        while ((bytesRead = sourceData.readRawData(&buf[0], 4096)) != 0)
        {
            if (bytesRead < 0)
                return Ori::Dlg::error(tr("Error reading from source file '%1'\n\n%2")
                   .arg(source.fileName()).append(source.errorString()));

            if (targetData.writeRawData(buf, bytesRead) < 0)
                return Ori::Dlg::error(tr("Error writing to target file '%1'\n\n%2")
                   .arg(target.fileName()).append(target.errorString()));
        }
    }
}

void BugHistory::setFocus()
{
    contentView->setFocus();
}

void BugHistory::scrollToEnd()
{
    QTextCursor c = contentView->textCursor();
    c.movePosition(QTextCursor::End);
    contentView->setTextCursor(c);
}

void BugHistory::commentAdded(int bugId)
{
    if (bugId == _id) scrollToEnd();
}

void BugHistory::setShowOnlyOpenedRelations(bool on)
{
    if (_showOnlyOpenedRelations == on) return;
    _showOnlyOpenedRelations = on;
    populate();
}






