#include "Relations.h"

#include "Db.h"

namespace Db::Relations {

class RelationsTableDef : public TableDef
{
public:
    RelationsTableDef() : TableDef("Relations") {}

    const QString id1 = ":id1";
    const QString id2 = ":id2";
    const QString created = ":created";

    const QString sqlInsert =
            "INSERT INTO Relations (Id1, Id2, Created) VALUES (:id1, :id2, :created)";

    const QString sqlDelete =
            "DELETE FROM Relations WHERE (Id1 = :id1 AND Id2 = :id2) OR (Id1 = :id2 AND Id2 = :id1)";

    QString sqlSelectById(int id) const {
        return QString("SELECT * FROM Relations WHERE Id1 = %1 OR Id2 = %1 ORDER BY Created").arg(id);
    }

    QString sqlSelectByIds(int id1, int id2) const {
        return QString("SELECT * FROM Relations WHERE (Id1 = %1 and Id2 = %2) "
                       "OR (Id1 = %2 and Id2 = %1)").arg(id1).arg(id2);
    }

    RelationItem recordToObject(const QSqlRecord& r) const
    {
        RelationItem item;
        item.id1 = r.value("Id1").toInt();
        item.id2 = r.value("Id2").toInt();
        return item;
    }
};

Q_GLOBAL_STATIC(RelationsTableDef, __table);


IntListResult get(int id)
{
    SelectQuery query(__table->sqlSelectById(id));
    if (query.isFailed())
        return IntListResult::fail(query.error());

    QList<int> ids;
    while (query.next())
    {
        auto item = __table->recordToObject(query.record());
        ids.append(item.id1 == id? item.id2: item.id1);
    }
    return IntListResult::ok(ids);
}

QString make(int id1, int id2)
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

    SelectQuery query(__table->sqlSelectByIds(id1, id2));
    if (query.isFailed())
        return query.error();
    if (query.next())
        return qApp->tr("There is an relation between #%1 and #%2 already.").arg(id1).arg(id2);

    QString res = ActionQuery(__table->sqlInsert)
            .param(__table->id1, id1)
            .param(__table->id2, id2)
            .param(__table->created, QDateTime::currentDateTime())
            .exec();
    if (!res.isEmpty())
        return qApp->tr("Failed to make new relation:\n\n%1").arg(res);
    return QString();
}

QString remove(int id1, int id2)
{
    QString res = ActionQuery(__table->sqlDelete)
            .param(__table->id1, id1)
            .param(__table->id2, id2)
            .exec();
    return !res.isEmpty() ? res : QString();
}

} // namespace Db::Relations
