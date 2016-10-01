#include "SqlHelpers.h"

//-----------------------------------------------------------------------------------------------

namespace SqlHelper {

QString errorText(const QSqlQuery &query, bool includeSql)
{
    return errorText(&query, includeSql);
}

QString errorText(const QSqlQuery *query, bool includeSql)
{
    QString text;
    if (includeSql)
        text = query->lastQuery() + "\n\n";
    return text + errorText(query->lastError());
}

QString errorText(const QSqlError &error)
{
    return QString("%1\n%2").arg(error.driverText()).arg(error.databaseText());
}

} // namespace SqlHelper

//-----------------------------------------------------------------------------------------------
