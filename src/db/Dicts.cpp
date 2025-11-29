#include "Dicts.h"

#include "Consts.h"

#include <QComboBox>
#include <QMap>
#include <QSqlField>
#include <QSqlRecord>
#include <QSqlTableModel>

namespace Db::Dicts {

typedef QMap<int, QString> DictCache;
typedef QHash<int, Style> DictStyle;

QMap<int, QSqlTableModel*> __tables;
QMap<int, DictCache*> __caches;
QHash<int, DictStyle*> __styles;

DictCache* cache(int dictId)
{
    return __caches.value(dictId);
}

QSqlTableModel* table(int dictId)
{
    return __tables.value(dictId);
}

QString value(int dictId, const QVariant& valId)
{
    auto cache = __caches.value(dictId);
    if (!cache)
        return valId.toString();

    int id = valId.toInt();
    return cache->contains(id) ? (*cache)[id] : valId.toString();
}

QString value(int dictId, int valId)
{
    auto cache = __caches.value(dictId);
    if (!cache)
        return QString::number(valId);

    return cache->contains(valId) ? (*cache)[valId] : QString::number(valId);
}

const Style* style(int dictId, int valId)
{
    auto styles = __styles.value(dictId);
    if (!styles)
        return nullptr;
        
    if (!styles->contains(valId))
        return nullptr;

    return &(*styles)[valId];
}

const QList<int>& dictIds()
{
    static QList<int> ids = { COL_CATEGORY, COL_SEVERITY, COL_PRIORITY, COL_STATUS, COL_REPEAT, COL_SOLUTION };
    return ids;
}

const QList<int>& dictIdsForStyling()
{
    static QList<int> ids = { COL_REPEAT, COL_CATEGORY, COL_SEVERITY, COL_PRIORITY, COL_SOLUTION, COL_STATUS };
    return ids;
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
    __tables.insert(dictId, tableModel);
    tableModel->setTable(table);
    tableModel->select();
}

void updateCache(int dictId)
{
    auto table = __tables.value(dictId);
    if (!table) return;

    auto cache = __caches.value(dictId);
    if (!cache)
    {
        cache = new DictCache;
        __caches.insert(dictId, cache);
    }

    cache->clear();
    for (int row = 0; row < table->rowCount(); row++)
    {
        auto record = table->record(row);
        cache->insert(record.field(DICT_COL_ID).value().toInt(),
                      record.field(DICT_COL_TITLE).value().toString());
    }
    
    if (dictId == COL_SEVERITY)
    {
        __styles.insert(dictId, new DictStyle {
            { SEVERITY_TODO, { .cellFontI = true } },
            { SEVERITY_ENHANCE, { .cellFontI = true } },
            { SEVERITY_BLUNDER, { .rowBackColor = QColor(255, 0, 0, 35), .cellFontB = true } },
            { SEVERITY_CRUSH, { .rowBackColor = QColor(255, 0, 0, 50), .cellFontB = true } },
            { SEVERITY_BLOCKER, { .rowBackColor = QColor(255, 0, 0, 75), .cellFontB = true } },
        });
    }
    else if (dictId == COL_PRIORITY)
    {
        __styles.insert(dictId, new DictStyle {
            { PRIORITY_MIN, { .rowTextColor = Qt::gray } },
            { PRIORITY_LOW, { .rowTextColor = Qt::gray } },
            { PRIORITY_HIGH, { .cellFontB = true } },
            { PRIORITY_URGENT, { .cellFontB = true } },
        });
    }
    else if (dictId == COL_STATUS)
    {
        __styles.insert(dictId, new DictStyle {
            { STATUS_SOLVED, { .rowBackColor = QColor(0, 255, 0, 50), .rowTextColor = QColor() } },
            { STATUS_CLOSED, { .rowBackColor = QColor(0, 0, 0, 50), .rowTextColor = QColor() } },
        });
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
    qDeleteAll(__tables);
    qDeleteAll(__caches);
    qDeleteAll(__styles);
    __tables.clear();
    __caches.clear();
    __styles.clear();
}

QComboBox* makeComboBox(int dictId)
{
    auto cb = new QComboBox;
    cb->setMaxVisibleItems(24);
    cb->setModel(table(dictId));
    cb->setModelColumn(DICT_COL_TITLE);
    cb->setCurrentIndex(0);
    return cb;
}

} // namespace Db::Dicts