#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include "../bugtypes.h"

class HistoryManager
{
public:
    BugHistoryResult get(int id) const;

    QString issuePropName(int propId) const;
};

#endif // HISTORY_MANAGER_H
