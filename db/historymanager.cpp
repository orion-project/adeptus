#include "historymanager.h"
#include "../sqlhelpers.h"
#include "../bugmanager.h" // TODO eliminate

#include <QApplication>

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

BugHistoryResult HistoryManager::get(int id) const
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

QString HistoryManager::issuePropName(int propId) const
{
    QString name = BugManager::columnTitle(propId);
    if (name.isEmpty())
        name = qApp->tr("Unknown property (%1)").arg(propId);
    return name;
}
