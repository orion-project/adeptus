#include "relationmanager.h"
#include "db.h"

RelationItem RelationsTableDef::recordToObject(const QSqlRecord& r) const
{
    RelationItem item;
    item.id1 = r.value("Id1").toInt();
    item.id2 = r.value("Id2").toInt();
    return item;
}

//-----------------------------------------------------------------------------


IntListResult RelationManager::get(int id) const
{
    SelectQuery query(_table.sqlSelectById(id));
    if (query.isFailed())
        return IntListResult::fail(query.error());

    QList<int> ids;
    while (query.next())
    {
        auto item = _table.recordToObject(query.record());
        ids.append(item.id1 == id? item.id2: item.id1);
    }
    return IntListResult::ok(ids);
}

QString RelationManager::make(int id1, int id2) const
{
    if (id1 == id2)
        return qApp->tr("Unable to relate an issue with itself");

    auto res1 = DB::issues().exists(id1);
    if (!res1.ok())
        return res1.error();
    else if (!res1.result())
        return qApp->tr("Issue not found (#%1)").arg(id1);

    auto res2 = DB::issues().exists(id2);
    if (!res2.ok())
        return res2.error();
    else if (!res2.result())
        return qApp->tr("Issue not found (#%1)").arg(id2);

    SelectQuery query(_table.sqlSelectByIds(id1, id2));
    if (query.isFailed())
        return query.error();
    if (query.next())
        return qApp->tr("There is an relation between #%1 and #%2 already.").arg(id1).arg(id2);

    QString res = _table.insertQuery()
                        .param(_table.id1, id1)
                        .param(_table.id2, id2)
                        .param(_table.created, QDateTime::currentDateTime())
                        .exec();
    if (!res.isEmpty())
        return qApp->tr("Failed to make new relation:\n\n%1").arg(res);
    return QString();
}

