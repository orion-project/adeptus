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

#define BUGS_FILES "Issue Databases (*.bugs);;All files (*.*)"

class BugManager
{
public:
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

    static bool isValid(int id) { return id > 0; }
    static bool isInvalid(int id) { return !isValid(id); }
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

namespace Db {
class Settings;
}

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
    virtual QString saveInternal(class Db::Settings&);
    virtual QString loadInternal(class Db::Settings&);
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
    QString saveInternal(class Db::Settings&) override;
private:
    int _id = 0;
    QString _title;
};


// class DbSettings
// {
// public:
//     DbSettings(bool transaction);
//     ~DbSettings();
//     QString saveSetting(const QString& name, const QVariant& value);
//     QString loadSetting(const QString& name, QVariant& value, const QVariant &def = QVariant());
//     QString lastError;
// private:
//     bool _transaction;
//     void rollback();
//     QString lastErrorStr();
// };


QVariant ptr2var(void *p);

template <typename T> T* var2obj(const QVariant& var)
{
    return reinterpret_cast<T*>(var.value<void*>());
}


bool checkResult(QWidget *parent, const QVariant& result, const QString& message);
bool checkResult(QWidget *parent, const QString& result, const QString& message);

#endif // BUGBASEMANAGER_H
