#include "Db.h"

#include "Dicts.h"

namespace DB {

const IssueManager& issues() { static IssueManager m; return m; }
const HistoryManager& history() { static HistoryManager m; return m; }

} // namespace DB

namespace Db {

#define CREATE_TABLE(table, columns) \
    res = createTable(table, columns); \
    if (!res.isEmpty()) { \
        __db.rollback(); \
        return res; }

#define INSERT_DICT_VALUE(table, id, value) \
    res = insertDictValue(table, id, value); \
    if (!res.isEmpty()) { \
        __db.rollback(); \
        return res; }

QString createTable(const QString &name, const QString &columns)
{
    QSqlQuery query;
    if (!query.exec(QString("create table if not exists %1 (%2)").arg(name).arg(columns)))
        return qApp->tr("Unable to create table %1.\n\n%2").arg(name.toUpper()).arg(SqlHelper::errorText(query));
    return "";
}

QString insertDictValue(const QString &table, int id, const QString &value)
{
    QSqlQuery query;
    if (!query.exec(QString("insert into %1 values (%2, '%3')").arg(table).arg(id).arg(value)))
        return qApp->tr("Unable to insert value %3 (%4) into dictionary %1.\n\n%2")
                .arg(QString(table).toUpper())
                .arg(SqlHelper::errorText(query))
                .arg(id)
                .arg(value);
    return "";
}

QSqlDatabase __db;

QSqlDatabase* db()
{
    return &__db;
}

void close()
{
    Db::Dicts::close();

    QString connection = __db.connectionName();
    __db.close();
    __db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection);
}

QString open(const QString &fileName)
{
    close();

    __db = QSqlDatabase::addDatabase("QSQLITE");
    __db.setDatabaseName(fileName);
    if (!__db.open())
        return qApp->tr("Unable to establish a database connection.\n\n%1")
                .arg(SqlHelper::errorText(__db.lastError()));

    QSqlQuery query;
    if (!query.exec("PRAGMA foreign_keys = ON;"))
        return qApp->tr("Failed to enable foreign keys.\n\n%1").arg(SqlHelper::errorText(query));

    QString res;

    if (!__db.transaction())
        return qApp->tr("Unable to begin transaction to create database structure.\n\n%1")
                .arg(SqlHelper::errorText(__db.lastError()));

    CREATE_TABLE(TABLE_CATEGORY, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_SEVERITY, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_PRIORITY, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_STATUS, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_SOLUTION, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_REPEAT, "Id integer primary key, Title varchar");

    CREATE_TABLE(TABLE_BUGS, "Id integer primary key, "                               // 0
                             "Summary varchar, "                                      // 1
                             "Extra varchar, "                                        // 2
                             "Category integer not null references Category(Id), "    // 3
                             "Severity integer not null references Severity(Id), "    // 4
                             "Priority integer not null references Priority(Id), "    // 5
                             "Repeat integer not null references Repeatability(Id), " // 6
                             "Status integer not null references Status(Id), "        // 7
                             "Solution integer not null references Solution(Id), "    // 8
                             "Created datetime not null, "                            // 9
                             "Updated datetime not null");                            // 10

    CREATE_TABLE(TABLE_HISTORY, "Issue integer not null references Issue(Id) on delete cascade, "
                                "EventNum integer not null, "
                                "EventPart integer not null, "
                                "ChangedParam integer not null default -1, "
                                "OldValue, "
                                "NewValue, "
                                "Comment varchar, "
                                "Moment datetime not null, "
                                "primary key (Issue, EventNum, EventPart)");


    CREATE_TABLE(TABLE_RELATIONS, "Id1 integer not null references Issue(Id) on delete cascade, "
                                  "Id2 integer not null references Issue(Id) on delete cascade, "
                                  "Created datetime not null, "
                                  "primary key (Id1, Id2)");

    CREATE_TABLE(TABLE_SETTINGS, "Name, Value");

    __db.commit();

    Dicts::open();

    return res;
}

QString create(const QString &fileName)
{
    if (QFileInfo(fileName) == QFileInfo(__db.databaseName()))
        close();

    if (QFile::exists(fileName))
        if (!QFile::remove(fileName))
            return qApp->tr("Unable to overwrite existing file. Probably file is locked.");

    QString res = open(fileName);
    if (!res.isEmpty())
        return res;

    if (!__db.transaction())
        return qApp->tr("Unable to begin transaction to insert default dictionary values.\n\n%1")
                .arg(SqlHelper::errorText(__db.lastError()));

    INSERT_DICT_VALUE(TABLE_CATEGORY, 0, "<none>");
    INSERT_DICT_VALUE(TABLE_CATEGORY, 100, "GUI");
    INSERT_DICT_VALUE(TABLE_CATEGORY, 200, "Input");
    INSERT_DICT_VALUE(TABLE_CATEGORY, 300, "Output");
    INSERT_DICT_VALUE(TABLE_CATEGORY, 400, "Processing");

    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_TODO,    "Todo");
    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_ENHANCE, "Enhancement");
    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_TEXT,    "Text");
    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_TRIVIAL, "Trivial");
    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_ERROR,   "Error");
    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_BLUNDER, "Blunder");
    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_CRUSH,   "Crush");
    INSERT_DICT_VALUE(TABLE_SEVERITY, SEVERITY_BLOCKER, "Blocker");

    INSERT_DICT_VALUE(TABLE_PRIORITY, PRIORITY_MIN,     "Minimal");
    INSERT_DICT_VALUE(TABLE_PRIORITY, PRIORITY_LOW,     "Low");
    INSERT_DICT_VALUE(TABLE_PRIORITY, PRIORITY_NORMAL,  "Normal");
    INSERT_DICT_VALUE(TABLE_PRIORITY, PRIORITY_HIGH,    "High");
    INSERT_DICT_VALUE(TABLE_PRIORITY, PRIORITY_URGENT,  "Urgent");

    INSERT_DICT_VALUE(TABLE_STATUS, STATUS_OPENED, "Opened");
    INSERT_DICT_VALUE(TABLE_STATUS, STATUS_SOLVED, "Solved");
    INSERT_DICT_VALUE(TABLE_STATUS, STATUS_CLOSED, "Closed");

    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_NONE, "None");
    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_FIXED, "Fixed");
    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_UNREPEAT, "Unrepeatable");
    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_IRRECOVER, "Irrecoverable");
    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_DUPLICATE, "Duplicate");
    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_REJECTED, "Rejected");
    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_SUSPENDED, "Suspended");
    INSERT_DICT_VALUE(TABLE_SOLUTION, SOLUTION_ABANDONED, "Abandoned");

    INSERT_DICT_VALUE(TABLE_REPEAT, 100, "Always");
    INSERT_DICT_VALUE(TABLE_REPEAT, 200, "Sometimes");
    INSERT_DICT_VALUE(TABLE_REPEAT, 300, "Unknown");

    __db.commit();

    Db::Dicts::open();

    return res;
}

QFileInfo attachedFile(const QString& fileName)
{
    QFileInfo file(__db.databaseName());
    file.setFile(file.absoluteDir().path() + '/' + file.completeBaseName() +
                 QStringLiteral(".files/") + fileName);
    return file;
}

} // namespace Db