#ifndef SQL_HELPERS_H
#define SQL_HELPERS_H

#include <QString>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

//-----------------------------------------------------------------------------------------------

namespace SqlHelper {

QString errorText(const QSqlQuery &query, bool includeSql = false);
QString errorText(const QSqlQuery *query, bool includeSql = false);
QString errorText(const QSqlError &error);

} // namespace SqlHelper

//-----------------------------------------------------------------------------------------------

namespace Ori {
namespace Sql {

class SelectQuery
{
public:
    SelectQuery(const QString& sql)
    {
        if (!_query.exec(sql))
            _error = SqlHelper::errorText(_query, true);
    }

    bool isFailed() const { return !_error.isEmpty(); }
    const QString& error() const { return _error; }
    const QSqlRecord& record() const { return _record; }

    bool next()
    {
        if (!_query.isSelect()) return false;
        bool ok =  _query.isValid() ? _query.next(): _query.first();
        if (ok) _record = _query.record();
        return ok;
    }

protected:
    QSqlQuery _query;
    QSqlRecord _record;

private:
    QString _error;
};

} // namespace Sql
} // namespace Ori

#endif // SQL_HELPERS_H
