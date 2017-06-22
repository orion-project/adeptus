#include "sqlbugprovider.h"

#include "bugmanager.h"
#include "sqlhelpers.h"

#include <QApplication>
#include <QSqlField>

//-----------------------------------------------------------------------------------------------

// TODO: get column values by name, not by index
BugInfo BugTableDef::recordToObject(const QSqlRecord& r) const
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

const BugTableDef& tableBugs()
{
    static BugTableDef table;
    return table;
}

//-----------------------------------------------------------------------------------------------

class BugHistoryQuery : public Ori::Sql::SelectQuery
{
public:
    BugHistoryQuery(int id) : Ori::Sql::SelectQuery(
        QString("select * from %1 where Issue = %2 order by EventNum").arg(TABLE_HISTORY).arg(id))
    {}

    int eventNum() const { return _record.field("EventNum").value().toInt(); }
    int eventPart() const { return _record.field("EventPart").value().toInt(); }
    int changedParam() const { return _record.field("ChangedParam").value().toInt(); }
    QVariant oldValue() const { return _record.field("OldValue").value(); }
    QVariant newValue() const { return _record.field("NewValue").value(); }
    QString comment() const { return _record.field("Comment").value().toString().trimmed(); }
    QDateTime moment() const { return _record.field("Moment").value().toDateTime(); }
};


class BugRelationsQuery : public Ori::Sql::SelectQuery
{
public:
    BugRelationsQuery(int id) : Ori::Sql::SelectQuery(
        QString("select * from %1 where Id1 = %2 or Id2 = %2 order by Created").arg(TABLE_RELATIONS).arg(id))
    {}

    int id1() const { return _record.value("Id1").toInt(); }
    int id2() const { return _record.value("Id2").toInt(); }
};

//-----------------------------------------------------------------------------------------------

BugHistoryResult SqlBugProvider::getHistory(int id)
{
    BugHistoryQuery query(id);
    if (query.isFailed())
        return BugHistoryResult::fail(query.error());

    BugHistoryItems history;
    BugHistoryItem item;
    while (query.next())
    {
        if (query.eventNum() > item.number)
        {
            if (item.isValid())
                history.append(item);

            item = BugHistoryItem();
            item.number = query.eventNum();
            item.moment = query.moment();
        }

        if (query.changedParam() >= 0)
            item.changedParams.append(BugHistoryItem::ChangedParam(
                                          query.changedParam(),
                                          query.oldValue(),
                                          query.newValue()));

        // Several rows in history table can correspond to a single event
        // (when several parameters were changed), but comment must be only one.
        QString c = query.comment();
        if (!c.isEmpty()) item.comment = c;
    }

    if (item.isValid())
        history.append(item);

    //for (const BugHistoryItem& it: history) qDebug() << it.str();

    return BugHistoryResult::ok(history);
}

IntListResult SqlBugProvider::getRelations(int id)
{
    BugRelationsQuery query(id);
    if (query.isFailed())
        return IntListResult::fail(query.error());

    QList<int> ids;
    while (query.next())
    {
        int id1 = query.id1();
        int id2 = query.id2();
        ids.append(id1 == id? id2: id1);
    }
    return IntListResult::ok(ids);
}

QString SqlBugProvider::bugParamName(int paramId)
{
    QString name = BugManager::columnTitle(paramId);
    if (name.isEmpty())
        name = qApp->tr("Unknown parameter (%1)").arg(paramId);
    return name;
}

//bool SqlBugProvider::isBugOpened(int status) { return status == STATUS_OPENED; }
//bool SqlBugProvider::isBugClosed(int status) { return status == STATUS_CLOSED; }
//bool SqlBugProvider::isBugSolved(int status) { return status == STATUS_SOLVED; }
