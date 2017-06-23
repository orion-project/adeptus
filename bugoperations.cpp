#include "bugoperations.h"

BugOperations* BugOperations::instance()
{
    static BugOperations obj;
    return &obj;
}

void BugOperations::raiseBugAdded(int id)
{
    emit bugAdded(id);
}

void BugOperations::raiseBugChanged(int id)
{
    emit bugChanged(id);
}

void BugOperations::raiseBugCommentAdded(int bugId)
{
    emit bugCommentAdded(bugId);
}
