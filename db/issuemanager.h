#ifndef ISSUE_MANAGER_H
#define ISSUE_MANAGER_H

#include "sqlhelpers.h"
#include "../bugtypes.h"
#include "../bugmanager.h" // TODO eliminate

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


class IssueManager
{
public:
    IssueResult get(int id) const;

    static bool isOpened(int status) { return status == STATUS_OPENED; }
    static bool isClosed(int status) { return status == STATUS_CLOSED; }
    static bool isSolved(int status) { return status == STATUS_SOLVED; }

    const IssueTableDef& table() const { return _table; }

private:
    IssueTableDef _table;
};

#endif // ISSUE_MANAGER_H
