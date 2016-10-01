#include "SqlBugProvider.h"

#include "bugmanager.h"
#include "SqlHelpers.h"

#include <QApplication>
#include <QSqlField>

//-----------------------------------------------------------------------------------------------

class BugRecord : public QSqlRecord
{
public:
    int id() const { return value(COL_ID).toInt(); }
    QString summary() const { return value(COL_SUMMARY).toString().trimmed(); }
    QString extra() const { return value(COL_EXTRA).toString().trimmed(); }
    int category() const { return value(COL_CATEGORY).toInt(); }
    int severity() const { return value(COL_SEVERITY).toInt(); }
    int priority() const { return value(COL_PRIORITY).toInt(); }
    int status() const { return value(COL_STATUS).toInt(); }
    int solution() const { return value(COL_SOLUTION).toInt(); }
    int repeat() const { return value(COL_REPEAT).toInt(); }
    QDateTime created() const { return value(COL_CREATED).toDateTime(); }
    QDateTime updated() const { return value(COL_UPDATED).toDateTime(); }
};


class BugQuery : public Ori::Sql::SelectQuery
{
public:
    BugQuery(int id) : Ori::Sql::SelectQuery(
        QString("SELECT * FROM %1 WHERE Id = %2").arg(TABLE_BUGS).arg(id))
    {}
};


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

BugResult SqlBugProvider::getBug(int id)
{
    BugQuery query(id);
    if (query.isFailed())
        return BugResult::fail(query.error());

    if (!query.next())
        return BugResult::fail(qApp->tr("Issue not found (#%1)").arg(id));

    return BugResult::ok(recordToBugInfo(query.record()));
}

BugInfo SqlBugProvider::recordToBugInfo(const QSqlRecord& record)
{
    const BugRecord& r = (const BugRecord&)record;
    BugInfo info;
    info.id = r.id();
    info.summary = r.summary();
    info.extra = r.extra();
    info.category = r.category();
    info.severity = r.severity();
    info.priority = r.priority();
    info.status = r.status();
    info.solution = r.solution();
    info.repeat = r.repeat();
    info.created = r.created();
    info.updated = r.updated();
    return info;
}

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

bool SqlBugProvider::isBugOpened(int status)
{
    return status == STATUS_OPENED;
}
