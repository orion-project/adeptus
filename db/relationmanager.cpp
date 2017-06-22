#include "relationmanager.h"

RelationItem RelationsTableDef::recordToObject(const QSqlRecord& r) const
{
    RelationItem item;
    item.id1 = r.value("Id1").toInt();
    item.id2 = r.value("Id2").toInt();
    return item;
}


IntListResult RelationManager::get(int issueId) const
{
    SelectQuery query(_table.sqlSelectById(issueId));
    if (query.isFailed())
        return IntListResult::fail(query.error());

    QList<int> ids;
    while (query.next())
    {
        auto item = _table.recordToObject(query.record());
        ids.append(item.id1 == issueId? item.id2: item.id1);
    }
    return IntListResult::ok(ids);
}
