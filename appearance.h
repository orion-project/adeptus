#ifndef APPEARANCE_H
#define APPEARANCE_H

#include <QColor>

class ColorProvider
{
public:
    static QColor solvedColor();
    static QColor solvedColorTransparent();
    static QColor closedColor();
    static QColor closedColorTransparent();
};

#endif // APPEARANCE_H
