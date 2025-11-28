#ifndef DICTS_H
#define DICTS_H

#include <QString>

class QComboBox;
class QSqlTableModel;

namespace Db::Dicts {

QString value(int dictId, const QVariant& valId);
QString value(int dictId, int valId);

QList<int> dictIds();

QSqlTableModel* table(int dictId);

void updateCache(int dictId);

QComboBox* makeComboBox(int dictId);

void open();
void close();

} // namespace Db::Dicts

#endif // DICTS_H
