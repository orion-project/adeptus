#ifndef SQL_BUG_PROVIDER_H
#define SQL_BUG_PROVIDER_H

#include "bugtypes.h"
#include "sqlhelpers.h"

using namespace Ori::Sql;

class IssueTableDef : public TableDef
{
public:
    IssueTableDef() : TableDef("Issue") {}

    QString sqlSelectById(int id) const {
        return QString("SELECT * FROM Issue WHERE Id = %1").arg(id);
    }

    IssueInfo recordToObject(const QSqlRecord& r) const;

//    DECLARE_COL(Id, QVariant::Int)
//    DECLARE_COL(Summary, QVariant::String)
//    DECLARE_COL(Extra, QVariant::String)
//    DECLARE_COL(Category, QVariant::Int)
//    DECLARE_COL(Severity, QVariant::Int)
//    DECLARE_COL(Priority, QVariant::Int)
//    DECLARE_COL(Repeat, QVariant::Int)
//    DECLARE_COL(Status, QVariant::Int)
//    DECLARE_COL(Solution, QVariant::Int)
//    DECLARE_COL(Created, QVariant::DateTime)
//    DECLARE_COL(Updated, QVariant::DateTime)
};

//-----------------------------------------------------------------------------------------------

class HistoryTableDef : public TableDef
{
public:
    HistoryTableDef() : TableDef("History") {}

    QString sqlSelectById(int id) const {
        return QString("SELECT * FROM History WHERE Issue = %1 ORDER BY EventNum").arg(id);
    }
};

//-----------------------------------------------------------------------------------------------

class RelationsTableDef : public TableDef
{
public:
    RelationsTableDef() : TableDef("Relations") {}

    QString sqlSelectById(int id) const {
        return QString("SELECT * FROM Relations WHERE Id1 = %1 OR Id2 = %1 ORDER BY Created").arg(id);
    }

    RelationItem recordToObject(const QSqlRecord& r) const;
};

//-----------------------------------------------------------------------------------------------

const IssueTableDef& tableIssues();
const HistoryTableDef& tableHistory();
const RelationsTableDef& tableRelations();

//-----------------------------------------------------------------------------------------------

class SqlBugProvider : public BugProvider
{
public:
    BugHistoryResult getHistory(int id) override;
    //IntListResult getRelations(int id) override;
    QString bugParamName(int paramId) override;

//    bool isBugOpened(int status) override;
//    bool isBugClosed(int status) override;
//    bool isBugSolved(int status) override;

    //static BugInfo recordToBugInfo(const QSqlRecord& record);

private:
    BugHistoryItem::ChangedParam getChangedParam(const QSqlRecord& record);
};

#endif // SQL_BUG_PROVIDER_H

