#ifndef SQL_BUG_PROVIDER_H
#define SQL_BUG_PROVIDER_H

#include "bugtypes.h"

#include <QSqlRecord>

class SqlBugProvider : public BugProvider
{
public:
    BugResult getBug(int id) override;
    BugHistoryResult getHistory(int id) override;
    IntListResult getRelations(int id) override;
    QString bugParamName(int paramId) override;
    bool isBugOpened(int status) override;

    static BugInfo recordToBugInfo(const QSqlRecord& record);

private:
    BugHistoryItem::ChangedParam getChangedParam(const QSqlRecord& record);
};

#endif // SQL_BUG_PROVIDER_H
