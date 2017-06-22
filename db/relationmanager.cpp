#include "relationmanager.h"
#include "../sqlbugprovider.h"

IntListResult RelationManager::get(int issueId) const
{
    SelectQuery query(tableRelations().sqlSelectById(issueId));
    if (query.isFailed())
        return IntListResult::fail(query.error());

    QList<int> ids;
    while (query.next())
    {
        auto item = tableRelations().recordToObject(query.record());
        ids.append(item.id1 == issueId? item.id2: item.id1);
    }
    return IntListResult::ok(ids);
}
