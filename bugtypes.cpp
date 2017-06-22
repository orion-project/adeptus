#include "bugtypes.h"

#include "bugmanager.h"

QString BugInfo::categoryTitle() { return BugManager::columnTitle(COL_CATEGORY); }
QString BugInfo::severityTitle() { return BugManager::columnTitle(COL_SEVERITY); }
QString BugInfo::priorityTitle() { return BugManager::columnTitle(COL_PRIORITY); }
QString BugInfo::statusTitle() { return BugManager::columnTitle(COL_STATUS); }
QString BugInfo::solutionTitle() { return BugManager::columnTitle(COL_SOLUTION); }
QString BugInfo::repeatTitle() { return BugManager::columnTitle(COL_REPEAT); }

QString BugInfo::categoryStr() const { return BugManager::dictionaryCash(COL_CATEGORY)->value(category); }
QString BugInfo::severityStr() const { return BugManager::dictionaryCash(COL_SEVERITY)->value(severity); }
QString BugInfo::priorityStr() const { return BugManager::dictionaryCash(COL_PRIORITY)->value(priority); }
QString BugInfo::statusStr() const { return BugManager::dictionaryCash(COL_STATUS)->value(status); }
QString BugInfo::solutionStr() const { return BugManager::dictionaryCash(COL_SOLUTION)->value(solution); }
QString BugInfo::repeatStr() const { return BugManager::dictionaryCash(COL_REPEAT)->value(repeat); }

bool BugInfo::isOpened() const { return BugManager::isOpened(status); }
bool BugInfo::isClosed() const { return BugManager::isClosed(status); }
bool BugInfo::isSolved() const { return BugManager::isSolved(status); }

//-----------------------------------------------------------------------------

QString BugHistoryItem::str() const
{
    QStringList params;
    params.append(QString("%1 @ %2").arg(number).arg(moment.toString()));

    for (const ChangedParam& p : changedParams)
        params.append("   " + p.str());

    params.append(comment);
    return params.join("\n");
}
