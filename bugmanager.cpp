#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QDir>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>

#include "bugmanager.h"
#include "db/sqlhelpers.h"

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

QSqlDatabase __db;

void BugManager::closeDatabase()
{
    BugManager::closeDictionaries();

    QString connection = __db.connectionName();
    __db.close();
    __db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection);
}

QString BugManager::openDatabase(const QString &fileName)
{
    closeDatabase();

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

    loadDictionaries();

    return res;
}

QString BugManager::newDatabase(const QString &fileName)
{
    if (QFileInfo(fileName) == QFileInfo(currentFile()))
        closeDatabase();

    if (QFile::exists(fileName))
        if (!QFile::remove(fileName))
            return qApp->tr("Unable to overwrite existing file. Probably file is locked.");

    QString res = openDatabase(fileName);
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

    INSERT_DICT_VALUE(TABLE_REPEAT, REPEAT_ALWAYS, "Always");
    INSERT_DICT_VALUE(TABLE_REPEAT, REPEAT_SOMETIMES, "Sometimes");
    INSERT_DICT_VALUE(TABLE_REPEAT, REPEAT_UNKNOWN, "Unknown");

    __db.commit();

    loadDictionaries();

    return res;
}

QString BugManager::currentFile()
{
    return __db.databaseName();
}

QString BugManager::createTable(const QString &name, const QString &columns)
{
    QSqlQuery query;
    if (!query.exec(QString("create table if not exists %1 (%2)").arg(name).arg(columns)))
        return qApp->tr("Unable to create table %1.\n\n%2").arg(name.toUpper()).arg(SqlHelper::errorText(query));
    return "";
}

QString BugManager::insertDictValue(const QString &table, int id, const QString &value)
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

QVariant BugManager::generateBugId()
{
    QSqlQuery query;
    if (!query.exec(QString("select max(Id) from %1").arg(TABLE_BUGS)))
        return SqlHelper::errorText(query);
    if (query.isSelect() && query.first())
        return query.record().value(0).toInt() + 1;
    return 1;
}

QVariant BugManager::generateEventId(int bugId)
{
    QSqlQuery query;
    if (!query.exec(QString("select max(EventNum) from %1 where Issue = %2")
                    .arg(TABLE_HISTORY).arg(bugId)))
        return SqlHelper::errorText(query);
    if (query.isSelect() && query.first())
        return query.record().value(0).toInt() + 1;
    return 1;
}

QVariant BugManager::generateEventPart(int bugId, int eventId)
{
    QSqlQuery query;
    if (!query.exec(QString("select max(EventPart) from %1 where Issue = %2 and EventNum = %3")
                    .arg(TABLE_HISTORY).arg(bugId).arg(eventId)))
        return SqlHelper::errorText(query);
    if (query.isSelect() && query.first())
        return query.record().value(0).toInt() + 1;
    return 1;
}

QString BugManager::dictionaryTableName(int dictId)
{
    switch (dictId)
    {
    case COL_CATEGORY: return TABLE_CATEGORY;
    case COL_SEVERITY: return TABLE_SEVERITY;
    case COL_PRIORITY: return TABLE_PRIORITY;
    case COL_STATUS: return TABLE_STATUS;
    case COL_REPEAT: return TABLE_REPEAT;
    case COL_SOLUTION: return TABLE_SOLUTION;
    }
    return "";
}

QMap<int, QSqlTableModel*> dictionaries;
QMap<int, DictionaryCash*> dictionaryCashes;

QSqlTableModel* BugManager::dictionary(int dictId)
{
    if (dictionaries.contains(dictId))
        return dictionaries.value(dictId);
    return NULL;
}

DictionaryCash *BugManager::dictionaryCash(int dictId)
{
    if (dictionaryCashes.contains(dictId))
        return dictionaryCashes.value(dictId);
    return NULL;
}

void BugManager::closeDictionaries()
{
    QSqlTableModel *table;
    foreach (table, dictionaries.values())
        delete table;
    dictionaries.clear();

    DictionaryCash *cash;
    foreach (cash, dictionaryCashes.values())
        delete cash;
    dictionaryCashes.clear();
}

void BugManager::loadDictionaries()
{
    closeDictionaries();

    loadDictionary(COL_STATUS);
    loadDictionary(COL_SOLUTION);
    loadDictionary(COL_SEVERITY);
    loadDictionary(COL_PRIORITY);
    loadDictionary(COL_REPEAT);
    loadDictionary(COL_CATEGORY);

    updateDictionaryCash(COL_STATUS);
    updateDictionaryCash(COL_SOLUTION);
    updateDictionaryCash(COL_SEVERITY);
    updateDictionaryCash(COL_PRIORITY);
    updateDictionaryCash(COL_REPEAT);
    updateDictionaryCash(COL_CATEGORY);
}

void BugManager::loadDictionary(int dictId)
{
    QString tableName = dictionaryTableName(dictId);
    if (tableName.isEmpty()) return;

    QSqlTableModel *tableModel = new QSqlTableModel;
    dictionaries.insert(dictId, tableModel);
    tableModel->setTable(tableName);
    tableModel->select();
}

void BugManager::updateDictionaryCash(int dictId)
{
    QSqlTableModel *table = dictionary(dictId);
    if (!table) return;

    DictionaryCash* cash = dictionaryCash(dictId);
    if (!cash)
    {
        cash = new DictionaryCash;
        dictionaryCashes.insert(dictId, cash);
    }

    cash->clear();
    for (int row = 0; row < table->rowCount(); row++)
    {
        QSqlRecord record = table->record(row);
        cash->insert(record.field(DICT_COL_ID).value().toInt(),
                     record.field(DICT_COL_TITLE).value().toString());
    }
}

QString BugManager::displayDictValue(int dictId, const QVariant &dictKey)
{
    QMap<int, QString>* dict = dictionaryCash(dictId);
    if (!dict)
        return dictKey.toString();

    if (!dict->contains(dictKey.toInt()))
        return dictKey.toString();

    return dict->value(dictKey.toInt());
}

QString BugManager::displayDateTime(const QVariant &value)
{
    return value.toDateTime().toString(Qt::SystemLocaleShortDate);
}

QString BugManager::addHistroyItem(int bugId, const QVariant& eventNum,
                                   const QDateTime &moment, int changedParam,
                                   const QVariant &oldValue, const QVariant &newValue)
{
    QSqlTableModel model;
    model.setTable(TABLE_HISTORY);

    return addHistroyItem(&model, bugId, eventNum, moment, changedParam, oldValue, newValue);
}

QString BugManager::addHistroyItem(QSqlTableModel *model,
                                   int bugId, const QVariant& eventNum,
                                   const QDateTime &moment, int changedParam,
                                   const QVariant &oldValue, const QVariant &newValue)
{
    QVariant eventPart = generateEventPart(bugId, eventNum.toInt());
    if (eventPart.type() == QVariant::String)
        return qApp->tr("Unable to generate history item id.\n\n%1").arg(eventPart.toString());

    QSqlRecord record;
    SqlHelper::addField(record, "Issue", QVariant::Int, bugId);
    SqlHelper::addField(record, "EventNum", QVariant::Int, eventNum);
    SqlHelper::addField(record, "EventPart", QVariant::Int, eventPart);
    SqlHelper::addField(record, "ChangedParam", QVariant::Int, changedParam);
    SqlHelper::addField(record, "OldValue", QVariant::Int, oldValue);
    SqlHelper::addField(record, "NewValue", QVariant::Int, newValue);
    SqlHelper::addField(record, "Moment", QVariant::DateTime, moment);

    if (!model->insertRecord(-1, record))
        return SqlHelper::errorText(model);

    return QString();
}

QString BugManager::addHistroyComment(int bugId, const QVariant& eventNum, const QDateTime &moment, const QString &comment)
{
    QVariant eventPart = generateEventPart(bugId, eventNum.toInt());
    if (eventPart.type() == QVariant::String)
        return qApp->tr("Unable to generate history item id.\n\n%1").arg(eventPart.toString());

    QSqlRecord record;
    SqlHelper::addField(record, "Issue", QVariant::Int, bugId);
    SqlHelper::addField(record, "EventNum", QVariant::Int, eventNum);
    SqlHelper::addField(record, "EventPart", QVariant::Int, eventPart);
    SqlHelper::addField(record, "ChangedParam", QVariant::Int, -1);
    SqlHelper::addField(record, "Moment", QVariant::DateTime, moment);
    SqlHelper::addField(record, "Comment", QVariant::String, comment);

    QSqlTableModel model;
    model.setTable(TABLE_HISTORY);

    if (!model.insertRecord(-1, record))
        return SqlHelper::errorText(model);

    return QString();
}

QString BugManager::countBugs(int& total, int& opened, int& displayed, const QString& filter)
{
    QSqlQuery query;
    if (!query.exec(QString("select count(Id) from %1").arg(TABLE_BUGS)))
        return qApp->tr("Unable to count total bugs: %1").arg(query.lastError().text());
    if (query.isSelect() && query.first())
        total = query.record().value(0).toInt();

    if (!query.exec(QString("select count(Id) from %1 where Status = %2")
                    .arg(TABLE_BUGS).arg(STATUS_OPENED)))
        return qApp->tr("Unable to count opened bugs: %1").arg(query.lastError().text());
    if (query.isSelect() && query.first())
        opened = query.record().value(0).toInt();

    if (!filter.isEmpty())
    {
        if (!query.exec(QString("select count(Id) from %1 where %2").arg(TABLE_BUGS).arg(filter)))
            return qApp->tr("Unable to count displayed bugs: %1").arg(query.lastError().text());
        if (query.isSelect() && query.first())
            displayed = query.record().value(0).toInt();
    }
    else
        displayed = total;

    return QString();
}

QString BugManager::debugGenerateIssues(QSqlTableModel*, int count)
{
    QSqlQuery query;
    for (int i = 0; i < count; i++)
    {
        QVariant id = BugManager::generateBugId();
        if (id.type() == QVariant::String)
            return qApp->tr("Unable to generate new issue id.\n\n%1").arg(id.toString());

        QString sql = QString("INSERT INTO %1 (Id, Summary, Extra, Category, Severity, "
                              "Priority, Repeat, Status, Solution, Created, Updated) "
                              "VALUES (%2, \"%3\", \"%4\", %5, %6, %7, %8, %9, %10, \"%11\", \"%11\")")
                      .arg(TABLE_BUGS)
                      .arg(id.toInt())
                      .arg("Lorem ipsum dolor sit amet, consectetuer adipiscing elit, "
                           "sed diam nonummy nibh euismod tincidunt ut laoreet dolore "
                           "magna aliquam erat volutpat.")
                      .arg("Ut wisi enim ad minim veniam, quis nostrud exerci tation "
                           "ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo "
                           "consequat. Duis autem vel eum iriure dolor in hendrerit in "
                           "vulputate velit esse molestie consequat, vel illum dolore eu "
                           "feugiat nulla facilisis at vero eros et accumsan et iusto odio "
                           "dignissim qui blandit praesent luptatum zzril delenit augue duis "
                           "dolore te feugait nulla facilisi. Nam liber tempor cum soluta nobis "
                           "eleifend option congue nihil imperdiet doming id quod mazim placerat "
                           "facer possim assum. Typi non habent claritatem insitam; est usus "
                           "legentis in iis qui facit eorum claritatem. Investigationes "
                           "demonstraverunt lectores legere me lius quod ii legunt saepius.")
                      .arg(CATEGORY_NONE)
                      .arg(500/*SEVERITY_ERROR*/) // TODO select random
                      .arg(PRIORITY_NORMAL)
                      .arg(REPEAT_ALWAYS)
                      .arg(STATUS_OPENED)
                      .arg(SOLUTION_NONE)
                      .arg(QDateTime::currentDateTime().toString());

        if (!query.exec(sql))
            return qApp->tr("Unable to insert new records: %1").arg(query.lastError().text());
    }
    return "";
}

QSqlRecord BugManager::getBugRecord(int id, QString& result)
{
    result.clear();
    QSqlQuery sql(QString("SELECT * FROM %1 WHERE Id = %2").arg(TABLE_BUGS).arg(id));
    if (!sql.exec())
    {
        result = SqlHelper::errorText(sql);
        return QSqlRecord();
    }
    if (!sql.isSelect() || !sql.first())
    {
        result = qApp->tr("Issue not found (#%1)").arg(id);
        return QSqlRecord();
    }
    return sql.record();
}

QString BugManager::deleteBug(int id)
{
    QSqlQuery sql(QString("DELETE FROM %1 WHERE Id = %2").arg(TABLE_BUGS).arg(id));
    if (!sql.exec())
        return SqlHelper::errorText(sql);
    return "";
}

QString BugManager::columnTitle(int colId)
{
    switch (colId)
    {
    case COL_ID: return qApp->translate("Column title", "ID");
    case COL_SUMMARY: return qApp->translate("Column title", "Summary");
    case COL_EXTRA: return qApp->translate("Column title", "Extra Information");
    case COL_CATEGORY: return qApp->translate("Column title", "Category");
    case COL_SEVERITY: return qApp->translate("Column title", "Severity");
    case COL_PRIORITY: return qApp->translate("Column title", "Priority");
    case COL_REPEAT: return qApp->translate("Column title", "Repeatability");
    case COL_STATUS: return qApp->translate("Column title", "Status");
    case COL_SOLUTION: return qApp->translate("Column title", "Solution");
    case COL_CREATED: return qApp->translate("Column title", "Created");
    case COL_UPDATED: return qApp->translate("Column title", "Updated");
    default: return QString();
    }
}

QString BugManager::operationTitle(int status)
{
    switch (status)
    {
    case STATUS_OPENED: return qApp->tr("Solve...");
    case STATUS_SOLVED: return qApp->tr("Close...");
    case STATUS_CLOSED: return qApp->tr("Reopen...");
    }
    return qApp->tr("Process..."); // stub name, no action
}

QList<int> BugManager::dictionaryIds()
{
    return { COL_CATEGORY, COL_SEVERITY, COL_PRIORITY, COL_STATUS, COL_SOLUTION, COL_REPEAT };
}

QString BugManager::makeRelation(int id1, int id2)
{
    if (id1 == id2)
        return qApp->tr("Unable to relate an issue with itself");
    QString result;
    getBugRecord(id1, result); if (!result.isEmpty()) return result;
    getBugRecord(id2, result); if (!result.isEmpty()) return result;
    QSqlQuery query;
    if (!query.exec(QString("select * from %1 where (Id1 = %2 and Id2 = %3) "
                            "or (Id1 = %3 and Id2 = %2)").arg(TABLE_RELATIONS).arg(id1).arg(id2)))
        return SqlHelper::errorText(query);
    if (query.isSelect() && query.first())
        return qApp->tr("There is an relation between #%1 and #%2 already.").arg(id1).arg(id2);
    QSqlRecord r;
    SqlHelper::addField(r, "Id1", id1);
    SqlHelper::addField(r, "Id2", id2);
    SqlHelper::addField(r, "Created", QDateTime::currentDateTime());
    QSqlTableModel table;
    table.setTable(TABLE_RELATIONS);
    if (!table.insertRecord(-1, r))
        return qApp->tr("Failed to make a new relation:\n\n%1").arg(SqlHelper::errorText(table));
    return QString();
}

QString BugManager::deleteRelation(int id1, int id2)
{
    QSqlQuery query;
    if (!query.exec(QString("delete from %1 where (Id1 = %2 and Id2 = %3) "
                            "or (Id1 = %3 and Id2 = %2)").arg(TABLE_RELATIONS).arg(id1).arg(id2)))
        return SqlHelper::errorText(query);
    return QString();
}

QFileInfo BugManager::fileInDatabaseFiles(const QString& fileName)
{
    QFileInfo file(currentFile());
    file.setFile(file.absoluteDir().path() % '/' % file.completeBaseName() %
                 QLatin1Literal(".files/") % fileName);
    return file;
}

//-----------------------------------------------------------------------------------------------

QString BugComparer::writeHistory(const BugInfo& oldValue, const BugInfo& newValue)
{
    QVariant eventNum = BugManager::generateEventId(oldValue.id);
    if (eventNum.type() == QVariant::String)
        return qApp->tr("Unable to generate history item number.\n\n%1").arg(eventNum.toString());

    QSqlTableModel model;
    model.setTable(TABLE_HISTORY);

    QString result;
    QDateTime moment = QDateTime::currentDateTime();

    if (oldValue.summary != newValue.summary)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_SUMMARY, oldValue.summary, newValue.summary);
    if (oldValue.extra != newValue.extra)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_EXTRA, oldValue.extra, newValue.extra);
    if (oldValue.category != newValue.category)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_CATEGORY, oldValue.category, newValue.category);
    if (oldValue.severity != newValue.severity)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_SEVERITY, oldValue.severity, newValue.severity);
    if (oldValue.priority != newValue.priority)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_PRIORITY, oldValue.priority, newValue.priority);
    if (oldValue.status != newValue.status)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_STATUS, oldValue.status, newValue.status);
    if (oldValue.solution != newValue.solution)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_SOLUTION, oldValue.solution, newValue.solution);
    if (oldValue.repeat != newValue.repeat)
        result += BugManager::addHistroyItem(&model, oldValue.id, eventNum, moment,
                                             COL_REPEAT, oldValue.repeat, newValue.repeat);
    return result;
}

//-----------------------------------------------------------------------------------------------

QComboBox* WidgetHelper::createDictionaryCombo(int dictId)
{
    QComboBox *combo = new QComboBox;
    combo->setMaxVisibleItems(24);
    combo->setModel(BugManager::dictionary(dictId));
    combo->setModelColumn(DICT_COL_TITLE);
    return combo;
}

void WidgetHelper::selectText(QComboBox *combo, const QString &value)
{
    for (int i = 0; i < combo->count(); i++)
        if (combo->itemText(i) == value)
        {
            combo->setCurrentIndex(i);
            return;
        }
}

void WidgetHelper::selectId(QComboBox *combo, const QVariant &value)
{
    QModelIndex startIndex = combo->model()->index(0, DICT_COL_ID);
    QModelIndexList values = combo->model()->match(startIndex, Qt::EditRole, value, 1, Qt::MatchExactly);
    if (!values.isEmpty())
        combo->setCurrentIndex(values.at(0).row());
}

QVariant WidgetHelper::selectedId(QComboBox *combo)
{
    return combo->model()->index(combo->currentIndex(), DICT_COL_ID).data();
}

//-----------------------------------------------------------------------------------------------

QString IssueFilter::getSql() const
{
    return check? QString("%1 %2 %3").arg(name).arg(condition).arg(value): QString();
}

//-----------------------------------------------------------------------------------------------

void IssueFilters::add(const QString& name, bool check, const QString& condition, int value)
{
    IssueFilter filter;
    filter.name = name;
    filter.check = check;
    filter.condition = condition;
    filter.value = value;
    filters.append(filter);
}

QString IssueFilters::getSql() const
{
    QString filter;
    for (int i = 0; i < filters.size(); i++)
    {
        QString condition = filters.at(i).getSql();
        if (!condition.isEmpty())
        {
            if (!filter.isEmpty()) filter += " and ";
            filter += condition;
        }
    }
    return filter;
}

QString IssueFilters::save()
{
    DbSettings dbs(true);
    if (!dbs.lastError.isEmpty())
        return qApp->tr("Unable to begin transaction for filters saving.\n\n%1").arg(dbs.lastError);

    return saveInternal(dbs);
}

QString IssueFilters::load()
{
    DbSettings dbs(false);

    return loadInternal(dbs);
}

QString IssueFilters::saveInternal(DbSettings& dbs)
{
    QString res, key, names;
    QString prefix = storagePrefix();

    for (int i = 0; i < filters.size(); i++)
    {
        const IssueFilter& filter = filters.at(i);
        key = QString("%1\\%2\\").arg(prefix, filter.name);

        res = dbs.saveSetting(key + "Check", filter.check);
        if (!res.isEmpty()) return res;

        res = dbs.saveSetting(key + "Condition", filter.condition);
        if (!res.isEmpty()) return res;

        res = dbs.saveSetting(key + "Value", filter.value);
        if (!res.isEmpty()) return res;

        names += filter.name + ";";
    }
    return dbs.saveSetting(QString("%1\\Names").arg(prefix), names);
}

QString IssueFilters::loadInternal(DbSettings& dbs)
{
    QVariant value;
    QString prefix = storagePrefix();
    QString key, res = dbs.loadSetting(QString("%1\\Names").arg(prefix), value, "");
    if (!res.isEmpty() || value.toString().isEmpty()) return res;
    QStringList names = value.toString().split(';', QString::SkipEmptyParts);
    for (const QString& name: names)
    {
        IssueFilter filter;
        filter.name = name;
        key = QString("%1\\%2\\").arg(prefix, filter.name);

        res = dbs.loadSetting(key + "Check", value, false);
        if (!res.isEmpty()) return res;
        filter.check = value.toBool();

        res = dbs.loadSetting(key + "Condition", value, "=");
        if (!res.isEmpty()) return res;
        filter.condition = value.toString();

        res = dbs.loadSetting(key + "Value", value, 100);
        if (!res.isEmpty()) return res;
        filter.value = value.toInt();

        filters.append(filter);
    }
    return QString();
}

//-----------------------------------------------------------------------------------------------

IntResult IssueFiltersPreset::generatePresetId()
{
    DbSettings dbs(false);
    QVariant value;
    QString res = dbs.loadSetting("FilterPresetMaxId", value, 0);
    if (!res.isEmpty()) return IntResult::fail(res);
    int id = value.toInt() + 1;
    res = dbs.saveSetting("FilterPresetMaxId", id);
    if (!res.isEmpty()) return IntResult::fail(res);
    return IntResult::ok(id);
}

IssueFiltersPreset::PresetsResult IssueFiltersPreset::loadPresets()
{
    QSqlQuery query;
    if (!query.exec(QString("select * from %1 where Name like 'FilterPreset\\%\\Title'").arg(TABLE_SETTINGS)))
        return PresetsResult::fail(SqlHelper::errorText(query, true));
    QMap<int, QString> presets;
    if (query.isSelect() && query.first())
        while (query.isValid())
        {
            QSqlRecord record = query.record();
            QStringList id_parts = record.value("Name").toString().split('\\');
            if (id_parts.size() > 1)
            {
                bool ok;
                int id = id_parts.at(1).toInt(&ok);
                if (ok) presets.insert(id, record.value("Value").toString());
            }
            query.next();
        }
    return PresetsResult::ok(presets);
}

QString IssueFiltersPreset::deletePreset(int id)
{
    QSqlQuery query;
    if (!query.exec(QString("delete from %1 where Name like 'FilterPreset\\%2\\%'").arg(TABLE_SETTINGS).arg(id)))
        return SqlHelper::errorText(query, true);
    return QString();
}

QString IssueFiltersPreset::storagePrefix() const
{
    return QString::fromLatin1("FilterPreset\\%1").arg(_id);
}

QString IssueFiltersPreset::saveInternal(DbSettings& dbs)
{
    QString prefix = storagePrefix();
    QString res = dbs.saveSetting(QString("%1\\Title").arg(prefix), _title);
    if (!res.isEmpty()) return res;

    return IssueFilters::saveInternal(dbs);
}

//-----------------------------------------------------------------------------------------------

DbSettings::DbSettings(bool transaction): _transaction(transaction)
{
    if (transaction)
        if (!__db.transaction())
            lastError = SqlHelper::errorText(__db.lastError());
}

DbSettings::~DbSettings()
{
    if (_transaction)
        __db.commit();
}

void DbSettings::rollback()
{
    if (_transaction)
    {
        lastError.clear();
        _transaction = false;
        if (!__db.rollback())
            lastError = SqlHelper::errorText(__db.lastError());
    }
}

QString DbSettings::lastErrorStr()
{
    return lastError.isEmpty()? QString(): "\n\n" + lastError;
}

QString DbSettings::saveSetting(const QString& name, const QVariant& value)
{
    QSqlQuery query;
    if (!query.exec(QString("select count(Name) from %1 where Name = '%2'").arg(TABLE_SETTINGS, name)))
    {
        rollback();
        return qApp->tr("Unable to save setting '%1': %2%3")
                .arg(name, SqlHelper::errorText(query), lastErrorStr());
    }
    QString sql;
    if (query.isSelect() && query.first() && query.record().value(0).toInt() == 0)
    {
        sql = QString("insert into %1 (Name, Value) values ('%2', '%3')")
                .arg(TABLE_SETTINGS, name, value.toString());
    }
    else
    {
        sql = QString("update %1 set Value = '%3' where Name = '%2'")
                .arg(TABLE_SETTINGS, name, value.toString());
    }
    if (!query.exec(sql))
    {
        rollback();
        return qApp->tr("Unable to save setting '%1': %2%3")
                .arg(name, SqlHelper::errorText(query), lastErrorStr());
    }
    return QString();
}

QString DbSettings::loadSetting(const QString& name, QVariant& value, const QVariant& def)
{
    QSqlQuery query;
    if (!query.exec(QString("select Value from %1 where Name = '%2'").arg(TABLE_SETTINGS, name)))
        return qApp->tr("Unable to load setting '%1': %2").arg(name, SqlHelper::errorText(query));
    if (query.isSelect() && query.first())
    {
        value = query.record().value(0);
        if (value.type() != QVariant::String)
            return qApp->tr("Setting '%1' has no proper type").arg(name);
        if (def.isValid() && def.type() != QVariant::String)
            if (!value.convert(def.type()))
                return qApp->tr("Setting '%1' has no proper type").arg(name);
    }
    else value = def;
    return QString();
}

//-----------------------------------------------------------------------------------------------

QVariant ptr2var(void *p)
{
    return QVariant::fromValue(p);
}

bool checkResult(QWidget *parent, const QVariant& result, const QString& message)
{
    if (result.type() == QVariant::String)
    {
        QMessageBox::critical(parent, qApp->applicationName(), message + "\n\n" + result.toString());
        return false;
    }
    return true;
}

bool checkResult(QWidget *parent, const QString& result, const QString& message)
{
    if (!result.isEmpty())
    {
        QMessageBox::critical(parent, qApp->applicationName(), message + "\n\n" + result);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------------------
QString DictManager::status(int id) { return BugManager::dictionaryCash(COL_STATUS)->value(id); }
QString DictManager::solution(int id) { return BugManager::dictionaryCash(COL_SOLUTION)->value(id); }


