#ifndef DICTS_H
#define DICTS_H

#include <QString>
#include <QColor>

class QComboBox;
class QSqlTableModel;

namespace Db::Dicts {

struct Style
{
    std::optional<QColor> rowBackColor, cellBackColor;
    std::optional<QColor> rowTextColor, cellTextColor;
    std::optional<bool> rowFontB, cellFontB;
    std::optional<bool> rowFontI, cellFontI;
    std::optional<bool> rowFontU, cellFontU;
    std::optional<bool> rowFontS, cellFontS;
};

QString value(int dictId, const QVariant& valId);
QString value(int dictId, int valId);
const Style* style(int dictId, int valId);

const QList<int>& dictIds();
const QList<int>& dictIdsForStyling();

QSqlTableModel* table(int dictId);

void updateCache(int dictId);

QComboBox* makeComboBox(int dictId);

void open();
void close();

} // namespace Db::Dicts

#endif // DICTS_H
