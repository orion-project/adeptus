#ifndef DICT_MANAGER_H
#define DICT_MANAGER_H

#include <QString>

class QComboBox;
class QSqlTableModel;

namespace Db::Dicts {

QString status(int valId);
QString solution(int valId);
QString value(int dictId, const QVariant& valId);
QString value(int dictId, int valId);

QSqlTableModel* table(int dictId);

void updateCache(int dictId);

QComboBox* makeComboBox(int dictId);

void open();
void close();

} // namespace Db::Dicts

#endif // DICT_MANAGER_H
