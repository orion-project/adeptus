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
