#include "markdown.h"
#include "bugmanager.h"

#include <QTextDocument>
#include <QImage>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QStringRef>
#endif

QString Markdown::hint()
{
    return QString(QStringLiteral("**<b>bold</b>**&nbsp;&nbsp;"
                                  "*<i>italic </i>*&nbsp;&nbsp;"
                                  "`<code>inline-code</code>`&nbsp;&nbsp;"
                                  "![title](image-or-file)&nbsp;&nbsp;"
                                  "![](image-with-preview)&nbsp;&nbsp;"
                                  "#issue-number"));
}

QString Markdown::process(const QString& s)
{
    // Markdown in Qt accepts html tags, but we want to show them as they are
    QString res = QString(s).replace("<", "&lt;").replace(">", "&gt;");

    bool ok;
    int offset;
    do { res = processImage(res, ok); } while (ok);
    do { res = processImage1(res, ok); } while (ok);
    do { res = processFile(res, ok); } while (ok);
    do { res = processBug(res, ok); } while (ok);
    do { res = processMarkdownResource(res, ok); } while (ok);
    offset = 0; do { res = processIssueNum(res, offset, ok); } while (ok);

    QTextDocument doc;
    doc.setMarkdown(res);
    return doc.toHtml();
}

QString Markdown::processImage(const QString& s, bool& ok)
{
    return processResource(s, QStringLiteral("Image"), BrowserCommands::showImage(), ok);
}

QString Markdown::processImage1(const QString& s, bool& ok)
{
    return processResource(s, QStringLiteral("Img"), BrowserCommands::showImage(), ok);
}

QString Markdown::processFile(const QString& s, bool& ok)
{
    return processResource(s, QStringLiteral("File"), BrowserCommands::getFile(), ok);
}

QString Markdown::processBug(const QString& s, bool& ok)
{
    return processResource(s, QStringLiteral("Bug"), BrowserCommands::showRelated(), ok);
}

QString Markdown::processIssueNum(const QString& s, int& offset, bool& ok)
{
    ok = false;
    int startPos = s.indexOf('#', offset, Qt::CaseInsensitive);
    if (startPos < 0) return s;

    ok = true;
    startPos++;
    int endPos = startPos;
    while (s[endPos].isDigit() && endPos < s.size())
        endPos++;
    if (endPos > startPos && (endPos == s.size() || s[endPos].isSpace() || s[endPos].isPunct()))
    {
        QString numStr = QStringRef(&s, startPos, endPos-startPos).toString();
        QString link = BrowserCommands::showRelated().format(numStr, '#'+numStr);
        offset = startPos-1 + link.size();

        QStringRef strBegin(&s, 0, startPos-1);
        QStringRef strEnd(&s, endPos, s.length() - endPos);
        return strBegin % link % strEnd;
    }

    offset = endPos;
    return s;
}

static QString makeImgLinkTag(const QString& link)
{
    QFileInfo fi = BugManager::fileInDatabaseFiles(link);
    QString fn = fi.absoluteFilePath();
    QImage img(fn);
    const int minSz = 250;
    const int maxSz = 300;
    int sz = qMax(img.width(), img.height());
    if (sz < maxSz)
        return QStringLiteral("<img src='%1'/>").arg(fn);
    QString side = img.width() > img.height() ? QStringLiteral("width") : QStringLiteral("height");
    QString tag = QStringLiteral("<img src='%1' %2=%3/>").arg(fn, side).arg(minSz);
    return BrowserCommands::showImage().format(link, tag);
}

QString Markdown::processResource(const QString& s, const QString& tag, const BrowserCommands::Command &cmd, bool& ok)
{
    ok = false;
    static QLatin1String tagStart("[[");
    static QLatin1String tagEnd("]]");

    int startPos = s.indexOf(tagStart % tag % ':', 0, Qt::CaseInsensitive);
    if (startPos < 0) return s;

    int resourceStartPos = startPos + tag.size() + 3;
    int endPos = s.indexOf(tagEnd, resourceStartPos+1);
    if (endPos < 0) return s;

    QStringRef strBegin(&s, 0, startPos);
    QStringRef strMid(&s, resourceStartPos, endPos - resourceStartPos);
    QStringRef strEnd(&s, endPos+2, s.length() - (endPos+2));

    QString resource = strMid.trimmed().toString();
    if (resource.isEmpty())
        return s;

    ok = true;

    QString linkTag;
    if (tag == QStringLiteral("Img") || tag == QStringLiteral("Image"))
        linkTag = makeImgLinkTag(resource);
    else linkTag = cmd.format(resource, resource);

    return strBegin % linkTag % strEnd;
}

QString Markdown::processMarkdownResource(const QString& s, bool& ok)
{
    ok = false;
    static QLatin1String tagStart("![");
    static QLatin1String tagMid("](");
    static QLatin1String tagEnd(")");

    int startPos = s.indexOf(tagStart, 0, Qt::CaseInsensitive);
    if (startPos < 0) return s;

    int altStartPos = startPos + 2;

    int midPos = s.indexOf(tagMid, altStartPos);
    if (midPos < 0) return s;

    int linkStartPos = midPos + 2;

    int endPos = s.indexOf(tagEnd, linkStartPos);
    if (endPos < 0) return s;

    QStringRef strBegin(&s, 0, startPos);
    QStringRef strAlt(&s, altStartPos, midPos - altStartPos);
    QStringRef strLink(&s, linkStartPos, endPos - linkStartPos);
    QStringRef strEnd(&s, endPos+1, s.length() - (endPos+1));

    QString link = strLink.trimmed().toString();
    if (link.isEmpty())
        return s;

    QString alt = strAlt.trimmed().toString();

    ok = true;
    QString linkTag;
    if (BrowserCommands::isSupportedImg(link))
        linkTag = alt.isEmpty() ? makeImgLinkTag(link) : BrowserCommands::showImage().format(link, alt);
    else linkTag = BrowserCommands::getFile().format(link, alt.isEmpty() ? link : alt);

    return strBegin % linkTag % strEnd;
}
