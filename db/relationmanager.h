#ifndef RELATION_MANAGER_H
#define RELATION_MANAGER_H

#include "../bugtypes.h"

class RelationManager
{
public:
    IntListResult get(int issueId) const;
};

#endif // RELATION_MANAGER_H
