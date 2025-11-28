#ifndef DB_H
#define DB_H

#include "historymanager.h"
#include "issuemanager.h"

namespace DB {

const IssueManager& issues();
const HistoryManager& history();

} // namespace Database

#endif // DB_H
