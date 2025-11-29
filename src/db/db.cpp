#include "Db.h"

#include "Dicts.h"

#include <QSysInfo>

#define DB_VERSION 2

namespace DB {

const IssueManager& issues() { static IssueManager m; return m; }
const HistoryManager& history() { static HistoryManager m; return m; }

} // namespace DB

namespace Db {

QString fileFilter()
{
    return "Issue Databases (*.bugs);;All files (*.*)";
}

#define CREATE_TABLE(table, columns) \
    if (auto res = createTable(table, columns); !res.isEmpty()) { \
        __db.rollback(); \
        close(); \
        return res; \
    } else qDebug() << "Table created:" << table;

#define INSERT_NAME_VALUE(table, id, value) \
    if (auto res = insertNameValue(table, id, value); !res.isEmpty()) { \
        __db.rollback(); \
        close(); \
        return res; \
    } else qDebug() << "Value inserted:" << table << id << value;

QString createTable(const QString &name, const QString &columns)
{
    QSqlQuery query;
    if (!query.exec(QString("create table if not exists %1 (%2)").arg(name).arg(columns)))
        return qApp->tr("Unable to create table %1.\n\n%2").arg(name.toUpper()).arg(SqlHelper::errorText(query));
    return "";
}

QString insertNameValue(const QString &table, const QVariant& id, const QVariant& value)
{
    auto res = ActionQuery(QString("insert into %1 values (:id, :value)").arg(table))
            .param(":id", id)
            .param(":value", value)
            .exec();
    if (!res.isEmpty())
        return qApp->tr("Unable to insert value %1=%2 into table %3.\n\n%4")
            .arg(id.toString())
            .arg(value.toString())
            .arg(table.toUpper())
            .arg(res);
    return "";
}

QSqlDatabase __db;

QSqlDatabase* db()
{
    return &__db;
}

QString lockFileName(const QString& fileName)
{
    return fileName + ".lock";
}

QString createLock(const QString &fileName)
{
    QFile lockFile(lockFileName(fileName));
    if (!lockFile.open(QIODeviceBase::WriteOnly))
        return qApp->tr("Failed to create lock file.\n\n%1").arg(lockFile.errorString());
    lockFile.write(QSysInfo::machineHostName().toUtf8());
    lockFile.write("\n");
    lockFile.write(QSysInfo::machineUniqueId());
    lockFile.write("\n");
    lockFile.write(QSysInfo::prettyProductName().toUtf8());
    lockFile.write("\n");
    return {};
}

void removeLock(const QString &fileName)
{
    auto lockFile = lockFileName(fileName);
    if (!QFile::exists(lockFile))
        qWarning() << "Lock file does not exists" << lockFile;
    else if (!QFile::remove(lockFile))
        qWarning() << "Failed to remove lock file when closing database" << lockFile;
}

void close()
{
    if (!__db.isOpen())
        return;
    
    Db::Dicts::close();
    
    auto fileName = __db.databaseName();
    auto connName = __db.connectionName();
    __db.close();
    __db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connName);
    removeLock(fileName);
}

QString connect(const QString &fileName)
{
    if (auto res = createLock(fileName); !res.isEmpty())
        return res;

    __db = QSqlDatabase::addDatabase("QSQLITE");
    __db.setDatabaseName(fileName);
    if (!__db.open())
    {
        removeLock(fileName);
        return qApp->tr("Unable to establish a database connection.\n\n%1")
                .arg(SqlHelper::errorText(__db.lastError()));
    }

    if (auto res = ActionQuery("PRAGMA foreign_keys = ON;").exec(); !res.isEmpty())
    {
        close();
        return qApp->tr("Failed to enable foreign keys.\n\n%1").arg(res);
    }

    return {};
}

QString open(const QString &fileName)
{
    close();

    if (QFile::exists(lockFileName(fileName)))
        return qApp->tr("The database is opened in another application instance");
        
    if (auto res = connect(fileName); !res.isEmpty())
        return res;

    Dicts::open();

    return {};
}

QString create(const QString &fileName)
{
    close();

    if (QFile::exists(fileName))
    {
        if (QFile::exists(lockFileName(fileName)))
            return qApp->tr("The existed database is opened in another instace of the application.");
    
        if (!QFile::remove(fileName))
            return qApp->tr("Unable to overwrite existing file. Probably file is locked.");
    }

    if (auto res = connect(fileName); !res.isEmpty())
        return res;

    if (!__db.transaction())
    {
        close();
        return qApp->tr("Unable to begin transaction to initialize database.\n\n%1")
                .arg(SqlHelper::errorText(__db.lastError()));
    }

    CREATE_TABLE(TABLE_CATEGORY, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_SEVERITY, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_PRIORITY, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_STATUS, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_SOLUTION, "Id integer primary key, Title varchar");
    CREATE_TABLE(TABLE_REPEAT, "Id integer primary key, Title varchar");

    CREATE_TABLE(TABLE_BUGS,
        "Id integer primary key, "                               // 0
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

    CREATE_TABLE(TABLE_HISTORY,
        "Issue integer not null references Issue(Id) on delete cascade, "
        "EventNum integer not null, "
        "EventPart integer not null, "
        "ChangedParam integer not null default -1, "
        "OldValue, "
        "NewValue, "
        "Comment varchar, "
        "Moment datetime not null, "
        "primary key (Issue, EventNum, EventPart)");


    CREATE_TABLE(TABLE_RELATIONS,
        "Id1 integer not null references Issue(Id) on delete cascade, "
        "Id2 integer not null references Issue(Id) on delete cascade, "
        "Created datetime not null, "
        "primary key (Id1, Id2)");
        
    CREATE_TABLE(TABLE_SETTINGS, "Name, Value");

    INSERT_NAME_VALUE(TABLE_CATEGORY, 0, "<none>");
    INSERT_NAME_VALUE(TABLE_CATEGORY, 100, "GUI");
    INSERT_NAME_VALUE(TABLE_CATEGORY, 200, "Input");
    INSERT_NAME_VALUE(TABLE_CATEGORY, 300, "Output");
    INSERT_NAME_VALUE(TABLE_CATEGORY, 400, "Processing");

    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_TODO,    "Todo");
    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_ENHANCE, "Enhancement");
    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_TEXT,    "Text");
    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_TRIVIAL, "Trivial");
    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_ERROR,   "Error");
    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_BLUNDER, "Blunder");
    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_CRUSH,   "Crush");
    INSERT_NAME_VALUE(TABLE_SEVERITY, SEVERITY_BLOCKER, "Blocker");

    INSERT_NAME_VALUE(TABLE_PRIORITY, PRIORITY_MIN,     "Minimal");
    INSERT_NAME_VALUE(TABLE_PRIORITY, PRIORITY_LOW,     "Low");
    INSERT_NAME_VALUE(TABLE_PRIORITY, PRIORITY_NORMAL,  "Normal");
    INSERT_NAME_VALUE(TABLE_PRIORITY, PRIORITY_HIGH,    "High");
    INSERT_NAME_VALUE(TABLE_PRIORITY, PRIORITY_URGENT,  "Urgent");

    INSERT_NAME_VALUE(TABLE_STATUS, STATUS_OPENED, "Opened");
    INSERT_NAME_VALUE(TABLE_STATUS, STATUS_SOLVED, "Solved");
    INSERT_NAME_VALUE(TABLE_STATUS, STATUS_CLOSED, "Closed");

    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_NONE, "None");
    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_FIXED, "Fixed");
    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_UNREPEAT, "Unrepeatable");
    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_IRRECOVER, "Irrecoverable");
    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_DUPLICATE, "Duplicate");
    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_REJECTED, "Rejected");
    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_SUSPENDED, "Suspended");
    INSERT_NAME_VALUE(TABLE_SOLUTION, SOLUTION_ABANDONED, "Abandoned");

    INSERT_NAME_VALUE(TABLE_REPEAT, 100, "Always");
    INSERT_NAME_VALUE(TABLE_REPEAT, 200, "Sometimes");
    INSERT_NAME_VALUE(TABLE_REPEAT, 300, "Unknown");
    
    INSERT_NAME_VALUE(TABLE_SETTINGS, "Version", DB_VERSION);
    
    __db.commit();

    Db::Dicts::open();

    return {};
}

QFileInfo attachedFile(const QString& fileName)
{
    QFileInfo file(__db.databaseName());
    file.setFile(file.absoluteDir().path() + '/' + file.completeBaseName() +
                 QStringLiteral(".files/") + fileName);
    return file;
}

} // namespace Db