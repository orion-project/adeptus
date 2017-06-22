#ifndef ISSUE_MANAGER_H
#define ISSUE_MANAGER_H

#include "../bugtypes.h"
#include "../bugmanager.h" // TODO eliminate

class IssueManager
{
public:
    IssueResult get(int id) const;

    static bool isOpened(int status) { return status == STATUS_OPENED; }
    static bool isClosed(int status) { return status == STATUS_CLOSED; }
    static bool isSolved(int status) { return status == STATUS_SOLVED; }
};

#endif // ISSUE_MANAGER_H
