#include "db.h"

namespace DB {

const IssueManager& issues() { static IssueManager m; return m; }
const RelationManager& relations() { static RelationManager m; return m; }
const HistoryManager& history() { static HistoryManager m; return m; }

} // namespace Database
