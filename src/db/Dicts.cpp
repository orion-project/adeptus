#include "Dicts.h"

#include "Consts.h"

#include <QComboBox>
#include <QMap>
#include <QSqlField>
#include <QSqlRecord>
#include <QSqlTableModel>

namespace Db::Dicts {

typedef QMap<int, QString> DictCache;

QMap<int, QSqlTableModel*> __dictTables;
QMap<int, DictCache*> __dictCaches;

static DictCache* cache(int dictId)
{
    return __dictCaches.value(dictId);
}

QSqlTableModel* table(int dictId)
{
    return __dictTables.value(dictId);
}

QString status(int valId)
{
    return cache(COL_STATUS)->value(valId);
}

QString solution(int valId)
{
    return cache(COL_SOLUTION)->value(valId);
}

QString value(int dictId, const QVariant& valId)
{
    auto cache = __dictCaches.value(dictId);
    if (!cache)
        return valId.toString();

    int id = valId.toInt();
    return cache->contains(id) ? (*cache)[id] : valId.toString();
}

QString value(int dictId, int valId)
{
    auto cache = __dictCaches.value(dictId);
    if (!cache)
        return QString::number(valId);

    return cache->contains(valId) ? (*cache)[valId] : QString::number(valId);
}

QList<int> dictIds()
{
    return { COL_CATEGORY, COL_SEVERITY, COL_PRIORITY, COL_STATUS, COL_REPEAT, COL_SOLUTION };
}

QString tableName(int id)
{
    switch (id)
    {
    case COL_CATEGORY: return TABLE_CATEGORY;
    case COL_SEVERITY: return TABLE_SEVERITY;
    case COL_PRIORITY: return TABLE_PRIORITY;
    case COL_STATUS: return TABLE_STATUS;
    case COL_REPEAT: return TABLE_REPEAT;
    case COL_SOLUTION: return TABLE_SOLUTION;
    }
    return "";
}

void openTable(int dictId)
{
    auto table = tableName(dictId);
    if (table.isEmpty()) return;

    auto tableModel = new QSqlTableModel;
    __dictTables.insert(dictId, tableModel);
    tableModel->setTable(table);
    tableModel->select();
}

void updateCache(int dictId)
{
    auto table = __dictTables.value(dictId);
    if (!table) return;

    auto cache = __dictCaches.value(dictId);
    if (!cache)
    {
        cache = new DictCache;
        __dictCaches.insert(dictId, cache);
    }

    cache->clear();
    for (int row = 0; row < table->rowCount(); row++)
    {
        auto record = table->record(row);
        cache->insert(record.field(DICT_COL_ID).value().toInt(),
                      record.field(DICT_COL_TITLE).value().toString());
    }
}

void open()
{
    close();

    for (int dictId : dictIds())
    {
        openTable(dictId);
        updateCache(dictId);
    }

}

void close()
{
    qDeleteAll(__dictTables);
    qDeleteAll(__dictCaches);
    __dictTables.clear();
    __dictCaches.clear();
}

QComboBox* makeComboBox(int dictId)
{
    auto combo = new QComboBox;
    combo->setMaxVisibleItems(24);
    combo->setModel(table(dictId));
    combo->setModelColumn(DICT_COL_TITLE);
    return combo;
}

} // namespace Db::Dicts