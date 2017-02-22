#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <QString>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class Markdown
{
public:
    static QString hint();
    static QLabel* makeHintLabel();
};

#endif // MARKDOWN_H
