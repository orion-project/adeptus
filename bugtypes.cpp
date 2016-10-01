#include "bugtypes.h"

QString BugHistoryItem::str() const
{
    QStringList params;
    params.append(QString("%1 @ %2").arg(number).arg(moment.toString()));

    for (const ChangedParam& p : changedParams)
        params.append("   " + p.str());

    params.append(comment);
    return params.join("\n");
}
