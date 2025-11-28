#include "bugtypes.h"

#include "bugmanager.h"
#include "db/Dicts.h"

QString BugInfo::categoryTitle() { return BugManager::columnTitle(COL_CATEGORY); }
QString BugInfo::severityTitle() { return BugManager::columnTitle(COL_SEVERITY); }
QString BugInfo::priorityTitle() { return BugManager::columnTitle(COL_PRIORITY); }
QString BugInfo::statusTitle() { return BugManager::columnTitle(COL_STATUS); }
QString BugInfo::solutionTitle() { return BugManager::columnTitle(COL_SOLUTION); }
QString BugInfo::repeatTitle() { return BugManager::columnTitle(COL_REPEAT); }

QString BugInfo::categoryStr() const { return Db::Dicts::value(COL_CATEGORY, category); }
QString BugInfo::severityStr() const { return Db::Dicts::value(COL_SEVERITY, severity); }
QString BugInfo::priorityStr() const { return Db::Dicts::value(COL_PRIORITY, priority); }
QString BugInfo::statusStr() const { return Db::Dicts::value(COL_STATUS, status); }
QString BugInfo::solutionStr() const { return Db::Dicts::value(COL_SOLUTION, solution); }
QString BugInfo::repeatStr() const { return Db::Dicts::value(COL_REPEAT, repeat); }

//-----------------------------------------------------------------------------

QString BugHistoryItem::str() const
{
    QVector<QString> params;
    params.append(QString("%1 @ %2").arg(number).arg(moment.toString()));

    for (const ChangedParam& p : changedParams)
        params.append("   " + p.str());

    params.append(comment);
    return params.join("\n");
}

QString BugHistoryItem::momentStr() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QLocale::system().toString(moment, QLocale::ShortFormat);
#else
    return moment.toString(Qt::SystemLocaleShortDate);
#endif
}
