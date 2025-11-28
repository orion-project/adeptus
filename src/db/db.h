#ifndef DB_H
#define DB_H

#include "historymanager.h"
#include "issuemanager.h"
#include "relationmanager.h"

namespace DB {

const IssueManager& issues();
const RelationManager& relations();
const HistoryManager& history();

} // namespace Database

#endif // DB_H
