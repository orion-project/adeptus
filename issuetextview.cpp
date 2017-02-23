#include "browsercommands.h"
#include "issuetextview.h"
#include "helpers/OriWidgets.h"

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
}

void IssueTextView::linkClicked(const QUrl& url)
{
    BrowserCommands::processCommand(url);
}
