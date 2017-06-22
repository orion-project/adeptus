#ifndef RELATION_MANAGER_H
#define RELATION_MANAGER_H

#include "sqlhelpers.h"
#include "../bugtypes.h"

using namespace Ori::Sql;


class RelationsTableDef : public TableDef
{
public:
    RelationsTableDef() : TableDef("Relations") {}

    const QString id1 = ":id1";
    const QString id2 = ":id2";
    const QString created = ":created";

    QString sqlSelectById(int id) const {
        return QString("SELECT * FROM Relations WHERE Id1 = %1 OR Id2 = %1 ORDER BY Created").arg(id);
    }

    QString sqlSelectByIds(int id1, int id2) const {
        return QString("SELECT * FROM Relations WHERE (Id1 = %1 and Id2 = %2) "
                       "OR (Id1 = %2 and Id2 = %1)").arg(id1).arg(id2);
    }
    
    ActionQuery insertQuery() const {
        return ActionQuery("INSERT INTO Relations (Id1, Id2, Created) VALUES (:id1, :id2, :created)");
    }

    RelationItem recordToObject(const QSqlRecord& r) const;
};


class RelationManager
{
public:
    IntListResult get(int id) const;
    QString make(int id1, int id2) const;

private:
    RelationsTableDef _table;
};

#endif // RELATION_MANAGER_H
