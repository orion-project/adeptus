#include "issuemanager.h"
#include "../sqlbugprovider.h"

#include <QApplication>

IssueResult IssueManager::get(int id) const
{
    SelectQuery query(tableIssues().sqlSelectById(id));
    if (query.isFailed())
        return BugResult::fail(query.error());

    if (!query.next())
        return BugResult::fail(qApp->tr("Issue not found (#%1)").arg(id));

    return BugResult::ok(tableIssues().recordToObject(query.record()));
}
