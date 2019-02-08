#include "issuemanager.h"

#include <QApplication>

// TODO: get column values by name, not by index
IssueInfo IssueTableDef::recordToObject(const QSqlRecord& r) const
{
    BugInfo info;
    info.id = r.value(COL_ID).toInt();
    info.summary = r.value(COL_SUMMARY).toString().trimmed();
    info.extra = r.value(COL_EXTRA).toString().trimmed();
    info.category = r.value(COL_CATEGORY).toInt();
    info.severity = r.value(COL_SEVERITY).toInt();
    info.priority = r.value(COL_PRIORITY).toInt();
    info.status = r.value(COL_STATUS).toInt();
    info.solution = r.value(COL_SOLUTION).toInt();
    info.repeat = r.value(COL_REPEAT).toInt();
    info.created = r.value(COL_CREATED).toDateTime();
    info.updated = r.value(COL_UPDATED).toDateTime();
    return info;
}

//-----------------------------------------------------------------------------

IssueResult IssueManager::get(int id) const
{
    SelectQuery query(_table.sqlSelectById(id));
    if (query.isFailed())
        return BugResult::fail(query.error());

    if (!query.next())
        return BugResult::fail(qApp->tr("Issue not found (#%1)").arg(id));

    return BugResult::ok(_table.recordToObject(query.record()));
}

BoolResult IssueManager::exists(int id) const
{
    SelectQuery query(_table.sqlExistsById(id));
    if (query.isFailed())
        return BoolResult::fail(query.error());

    return BoolResult::ok(query.next());
}

QString IssueManager::remove(int id) const
{
    QString res = ActionQuery(_table.sqlDelete(id)).exec();
    return !res.isEmpty() ? res : QString();
}
