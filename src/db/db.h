#ifndef DB_H
#define DB_H

#include "historymanager.h"
#include "issuemanager.h"

namespace DB {

const IssueManager& issues();
const HistoryManager& history();

} // namespace DB

namespace Db {

QSqlDatabase* db();

QString fileFilter();

QString open(const QString &fileName);
QString create(const QString &fileName);
void close();

QFileInfo attachedFile(const QString& fileName);

} // namespace Db

#endif // DB_H
