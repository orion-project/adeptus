#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <QString>

#include "browsercommands.h"

class Markdown
{
public:
    static QString hint();

    static QString process(const QString& s);

private:
    static QString processBold(const QString& s, bool& ok);
    static QString processItalic(const QString& s, bool& ok);
    static QString processImage(const QString& s, bool& ok);
    static QString processImage1(const QString& s, bool& ok);
    static QString processFile(const QString& s, bool& ok);
    static QString processBug(const QString& s, bool& ok);
    static QString processResource(const QString& s, const QString& tag, const BrowserCommands::Command& cmd, bool& ok);
};

#endif // MARKDOWN_H

