#include "markdown.h"

#include <QLabel>

QString Markdown::hint()
{
    return QString(QStringLiteral("'''<b>bold</b>'''&nbsp;&nbsp;"
                                  "''<i>italic</i>''&nbsp;&nbsp;"
                                  "[[Image:file-name]]&nbsp;&nbsp;"
                                  "[[File:file-name]]&nbsp;&nbsp;"
                                  "[[Bug:bug-number]]"));
}

QLabel* Markdown::makeHintLabel()
{
    auto label = new QLabel(hint());
    auto palette = label->palette();
    auto color = palette.color(QPalette::WindowText);
    color.setAlpha(80);
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);
    return label;
}

QString Markdown::process(const QString& s)
{
    bool ok;
    QString res = s;
    do { res = processBold(res, ok); } while (ok);
    do { res = processItalic(res, ok); } while (ok);
    do { res = processImage(res, ok); } while (ok);
    do { res = processFile(res, ok); } while (ok);
    do { res = processBug(res, ok); } while (ok);
    return res;
}

QString Markdown::processBold(const QString& s, bool& ok)
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

QString Markdown::processItalic(const QString& s, bool& ok)
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

QString Markdown::processImage(const QString& s, bool& ok)
{
    return processResource(s, QStringLiteral("Image"), BrowserCommands::showImage(), ok);
}

QString Markdown::processFile(const QString& s, bool& ok)
{
    return processResource(s, QStringLiteral("File"), BrowserCommands::getFile(), ok);
}

QString Markdown::processBug(const QString& s, bool& ok)
{
    return processResource(s, QStringLiteral("Bug"), BrowserCommands::showRelated(), ok);
}

QString Markdown::processResource(const QString& s, const QString& tag, const BrowserCommands::CommandDef &cmd, bool& ok)
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

