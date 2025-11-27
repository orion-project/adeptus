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

    QString sqlExistsById(int id) const {
        return QString("SELECT Id FROM Issue WHERE Id = %1").arg(id);
    }

    QString sqlDelete(int id) const {
        return QString("DELETE FROM Issue WHERE Id = %1").arg(id);
    }

    IssueInfo recordToObject(const QSqlRecord& r) const;
};


class IssueManager
{
public:
    IssueResult get(int id) const;
    BoolResult exists(int id) const;
    QString remove(int id) const;

    static bool isOpened(int status) { return status == STATUS_OPENED; }
    static bool isClosed(int status) { return status == STATUS_CLOSED; }
    static bool isSolved(int status) { return status == STATUS_SOLVED; }

    const IssueTableDef& table() const { return _table; }

private:
    IssueTableDef _table;
};

#endif // ISSUE_MANAGER_H
