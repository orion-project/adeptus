#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <QString>

#include "browsercommands.h"

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class Markdown
{
public:
    static QString hint();
    static QLabel* makeHintLabel();

    static QString process(const QString& s);

private:
    static QString processBold(const QString& s, bool& ok);
    static QString processItalic(const QString& s, bool& ok);
    static QString processImage(const QString& s, bool& ok);
    static QString processFile(const QString& s, bool& ok);
    static QString processBug(const QString& s, bool& ok);
    static QString processResource(const QString& s, const QString& tag, const BrowserCommands::Command& cmd, bool& ok);
};

#endif // MARKDOWN_H
