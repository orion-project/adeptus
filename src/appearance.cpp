#include "appearance.h"
#include "helpers/OriTools.h"

#include <QApplication>
#include <QPalette>

inline QColor stdColor(QPalette::ColorRole cr)
{
    return qApp->palette().color(cr);
}

QColor ColorProvider::solvedColor()
{
    return Ori::Color::blend(stdColor(QPalette::Base), Qt::green, 0.2);
}

QColor ColorProvider::solvedColorTransparent()
{
    return QColor(0, 255, 0, 50);
}

QColor ColorProvider::closedColor()
{
    return Ori::Color::blend(stdColor(QPalette::Base), Qt::black, 0.15);
}

QColor ColorProvider::closedColorTransparent()
{
    return QColor(0, 0, 0, 50);
}
