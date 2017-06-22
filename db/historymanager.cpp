#include "historymanager.h"
#include "../bugmanager.h" // TODO eliminate

#include <QApplication>

class HistoryQuery : public SelectQuery
{
public:
    HistoryQuery(int id) : SelectQuery(
        QString("select * from History where Issue = %1 order by EventNum").arg(id))
    {}

    int eventNum() const { return _record.field("EventNum").value().toInt(); }
    int eventPart() const { return _record.field("EventPart").value().toInt(); }
    int changedParam() const { return _record.field("ChangedParam").value().toInt(); }
    QVariant oldValue() const { return _record.field("OldValue").value(); }
    QVariant newValue() const { return _record.field("NewValue").value(); }
    QString comment() const { return _record.field("Comment").value().toString().trimmed(); }
    QDateTime moment() const { return _record.field("Moment").value().toDateTime(); }
};


HistoryResult HistoryManager::get(int id) const
{
    HistoryQuery query(id);
    if (query.isFailed())
        return HistoryResult::fail(query.error());

    HistoryItems history;
    HistoryItem item;
    while (query.next())
    {
        if (query.eventNum() > item.number)
        {
            if (item.isValid())
                history.append(item);

            item = HistoryItem();
            item.number = query.eventNum();
            item.moment = query.moment();
        }

        if (query.changedParam() >= 0)
            item.changedParams.append(HistoryItem::ChangedParam(
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

    return HistoryResult::ok(history);
}

QString HistoryManager::issuePropName(int propId) const
{
    QString name = BugManager::columnTitle(propId);
    if (name.isEmpty())
        name = qApp->tr("Unknown property (%1)").arg(propId);
    return name;
}
