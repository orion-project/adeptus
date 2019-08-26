#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include "sqlhelpers.h"
#include "../db_old/bugtypes.h"

using namespace Ori::Sql;


//class HistoryTableDef : public TableDef
//{
//public:
//    HistoryTableDef() : TableDef("History") {}

//    QString sqlSelectById(int id) const {
//        return QString("SELECT * FROM History WHERE Issue = %1 ORDER BY EventNum").arg(id);
//    }
//};


class HistoryManager
{
public:
    HistoryResult get(int id) const;

    QString issuePropName(int propId) const;

//private:
//    HistoryTableDef _table;
};

#endif // HISTORY_MANAGER_H
