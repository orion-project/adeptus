#include "browsercommands.h"
#include "issuetextview.h"
#include "helpers/OriWidgets.h"

#include <QDesktopServices>
#include <QToolTip>

QString sanitizeHtml(const QString& s)
{
    return QString(s).replace("<", "&lt;").replace(">", "&gt;").replace("\n\n", "<p>").replace("\n", "<br>");
}

IssueTextView::IssueTextView(QWidget *parent) : QTextBrowser(parent)
{
    Ori::Gui::adjustFont(this);
    setReadOnly(true);
    setOpenLinks(false);
    connect(this, &QTextBrowser::anchorClicked, this, &IssueTextView::linkClicked);
    connect(this, &QTextBrowser::highlighted, this, &IssueTextView::linkHovered);
}

void IssueTextView::linkClicked(const QUrl& url)
{
    if (BrowserCommands::isCommand(url))
    {
        QString cmd = BrowserCommands::getCommand(url);

        // Some basic commands whose processing makes sense in dialogs (e.g. issue editor)
        if (BrowserCommands::showRelated() == cmd)
            return BrowserCommands::showRelated().exec(url);

        if (BrowserCommands::showImage() == cmd)
            return BrowserCommands::showImage().exec(url);

        if (BrowserCommands::getFile() == cmd)
            return BrowserCommands::getFile().exec(url);

        // Rest of commands should be processed in wider context by parent widget (e.g. history browser)
        emit processCommand(cmd, url);
    }
    else if (url.scheme().startsWith("http"))
        QDesktopServices::openUrl(url);
}

void IssueTextView::linkHovered(const QUrl& url)
{
    if (BrowserCommands::isCommand(url))
    {
        QString tooltip = BrowserCommands::getHint(url);
        if (!tooltip.isEmpty())
        {
            QToolTip::showText(QCursor::pos(), tooltip);
            return;
        }
    }
    else if (url.scheme().startsWith("http"))
    {
        QToolTip::showText(QCursor::pos(), url.toString());
        return;
    }
    QToolTip::hideText();
}
