#ifndef BUGBASEMANAGER_H
#define BUGBASEMANAGER_H

#include <QMap>
#include <QDateTime>
#include <QVariant>
#include <QVector>
#include <QFileInfo>

#include "bugtypes.h"
#include "db/Consts.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QString;
class QSqlError;
class QSqlQuery;
class QSqlRecord;
class QSqlTableModel;
class QVariant;
class QWidget;
QT_END_NAMESPACE

#define CATEGORY_NONE   0

#define STATUS_OPENED   100
#define STATUS_SOLVED   200
#define STATUS_CLOSED   300

#define PRIORITY_MIN    100
#define PRIORITY_LOW    200
#define PRIORITY_NORMAL 300
#define PRIORITY_HIGH   400
#define PRIORITY_URGENT 500

#define SEVERITY_TODO       100
#define SEVERITY_ENHANCE    200
#define SEVERITY_TEXT       300
#define SEVERITY_TRIVIAL    400
#define SEVERITY_ERROR      500
#define SEVERITY_BLUNDER    600
#define SEVERITY_CRUSH      700
#define SEVERITY_BLOCKER    800

#define SOLUTION_NONE       0
#define SOLUTION_FIXED      100
#define SOLUTION_UNREPEAT   200
#define SOLUTION_IRRECOVER  300
#define SOLUTION_DUPLICATE  400
#define SOLUTION_REJECTED   500
#define SOLUTION_SUSPENDED  600
#define SOLUTION_ABANDONED  700

#define BUGS_FILES "Issue Databases (*.bugs);;All files (*.*)"

class BugManager
{
public:
    static QString openDatabase(const QString &fileName);
    static QString newDatabase(const QString &fileName);
    static void closeDatabase();
    static QString currentFile();
    static QVariant generateBugId();
    static QVariant generateEventId(int bugId);
    static QVariant generateEventPart(int bugId, int eventId);
    static QString displayDateTime(const QVariant &value);
    static QString addHistroyComment(int bugId, const QVariant &eventNum,
                                     const QDateTime &moment, const QString &comment);
    static QString addHistroyItem(int bugId, const QVariant& eventNum,
                                  const QDateTime &moment, int changedParam,
                                  const QVariant &oldValue, const QVariant &newValue);
    static QString addHistroyItem(QSqlTableModel *model,
                                  int bugId, const QVariant& eventNum,
                                  const QDateTime &moment, int changedParam,
                                  const QVariant &oldValue, const QVariant &newValue);
    static QString countBugs(int &total, int &opened, int& displayed, const QString& filter);

    static QString columnTitle(int colId);
    static QString operationTitle(int status);

    static QFileInfo fileInDatabaseFiles(const QString& fileName);
    static bool isValid(int id) { return id > 0; }
    static bool isInvalid(int id) { return !isValid(id); }

private:
    static QString createTable(const QString &name, const QString &columns);
    static QString insertDictValue(const QString &table, int id, const QString &value);
};


class BugComparer
{
public:
    static QString writeHistory(const BugInfo& oldValue, const BugInfo& newValue);
};


class WidgetHelper
{
public:
    static void selectText(QComboBox *combo, const QString &value);
    static void selectId(QComboBox *combo, const QVariant &value);
    static QVariant selectedId(QComboBox *combo);
};


struct IssueFilter
{
    bool check;
    int value;
    QString name;
    QString condition;
    QString getSql() const;
};


class IssueFilters
{
public:
    QVector<IssueFilter> filters;
    void add(const QString& name, bool check, const QString& condition, int value);
    QString getSql() const;
    QString save();
    QString load();
protected:
    virtual QString storagePrefix() const { return "Filter"; }
    virtual QString saveInternal(class DbSettings&);
    virtual QString loadInternal(class DbSettings&);
};


class IssueFiltersPreset : public IssueFilters
{
public:
    IssueFiltersPreset(int id): _id(id) {}
    IssueFiltersPreset(int id, const QString& title): _id(id), _title(title) {}
    static IntResult generatePresetId();
    typedef QueryResult<QMap<int, QString>> PresetsResult;
    static PresetsResult loadPresets();
    static QString deletePreset(int id);
protected:
    QString storagePrefix() const override;
    QString saveInternal(class DbSettings&) override;
private:
    int _id = 0;
    QString _title;
};


class DbSettings
{
public:
    DbSettings(bool transaction);
    ~DbSettings();
    QString saveSetting(const QString& name, const QVariant& value);
    QString loadSetting(const QString& name, QVariant& value, const QVariant &def = QVariant());
    QString lastError;
private:
    bool _transaction;
    void rollback();
    QString lastErrorStr();
};


QVariant ptr2var(void *p);

template <typename T> T* var2obj(const QVariant& var)
{
    return reinterpret_cast<T*>(var.value<void*>());
}


bool checkResult(QWidget *parent, const QVariant& result, const QString& message);
bool checkResult(QWidget *parent, const QString& result, const QString& message);

#endif // BUGBASEMANAGER_H
