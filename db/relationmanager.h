#ifndef RELATION_MANAGER_H
#define RELATION_MANAGER_H

#include "sqlhelpers.h"
#include "../bugtypes.h"

using namespace Ori::Sql;


class RelationsTableDef : public TableDef
{
public:
    RelationsTableDef() : TableDef("Relations") {}

    QString sqlSelectById(int id) const {
        return QString("SELECT * FROM Relations WHERE Id1 = %1 OR Id2 = %1 ORDER BY Created").arg(id);
    }

    RelationItem recordToObject(const QSqlRecord& r) const;
};


class RelationManager
{
public:
    IntListResult get(int issueId) const;

private:
    RelationsTableDef _table;
};

#endif // RELATION_MANAGER_H
