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
#include "browsercommands.h"
#include "bughistory.h"
#include "bugmanager.h"
#include "bugsolver.h"
#include "bugeditor.h"
#include "bugoperations.h"
#include "markdown.h"
#include "SqlBugProvider.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"

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
            "<p>" % BrowserCommands::addComment().format(tr("Append comment")) %
             "&nbsp;&nbsp;&nbsp;" %
             BrowserCommands::makeRelation().format(tr("Make relation"));
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
            .arg(BrowserCommands::copySummary().format(QString("#%1").arg(_id))).arg(sanitizeHtml(_summary))
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
        content += QString("<tr class='extra'><td>%1</td></tr>").arg(Markdown::process(sanitizeHtml(extra)));

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
                BrowserCommands::showRelated().format(relatedId, sanitizeHtml(bug.summary));
            moment = formatMoment(bug.created);
            command = BrowserCommands::delRelated().format(_id, relatedId, "<img src=':/tools/delete'>");
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
        .arg(BrowserCommands::makeRelation().format("<img src=':/tools/plus'>"));


    return title + content;
}

QString BugHistory::formatRelationsCount(int countOpened)
{
    auto countAll = QString::number(_relatedIds.size());
    auto countOpen = QString::number(countOpened);
    return tr("%1 %2, %3 %4")
        .arg(BrowserCommands::showAllRelations().format(tr("all:", "Show all relations command")))
        .arg(_showOnlyOpenedRelations? countAll: ("<b>" % countAll % "</b>"))
        .arg(BrowserCommands::showOpenedRelations().format(tr("opened:", "Show opened relations command")))
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
                    .arg(Markdown::process(sanitizeHtml(item.comment)));

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
            BrowserCommands::showText().format(_changedTextIndex, paramName));
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
        if (BrowserCommands::copySummary() == cmd)
            QApplication::clipboard()->setText(QString("#%1: %2").arg(_id).arg(_summary));

        else if (BrowserCommands::addComment() == cmd)
           emit operationRequest(BugManager::Operation_Comment, 0);

        else if (BrowserCommands::makeRelation() == cmd)
            emit operationRequest(BugManager::Operation_MakeRelation, 0);

        else if (BrowserCommands::showText() == cmd)
            showChangedText(BrowserCommands::showText().arg1Int(url));

        else if (BrowserCommands::showAllRelations() == cmd)
            setShowOnlyOpenedRelations(false);

        else if (BrowserCommands::showOpenedRelations() == cmd)
            setShowOnlyOpenedRelations(true);

        else
            BrowserCommands::processCommand(url);
    }
}

void BugHistory::linkHovered(const class QUrl& url)
{
    QString tooltip = BrowserCommands::getHint(url);
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
