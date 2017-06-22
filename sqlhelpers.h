#ifndef SQL_HELPERS_H
#define SQL_HELPERS_H

#include <QString>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>

//-----------------------------------------------------------------------------------------------

namespace SqlHelper {

void addField(QSqlRecord &record, const QString &name, QVariant::Type type, const QVariant &value);
void addField(QSqlRecord &record, const QString &name, const QVariant &value);

QString errorText(const QSqlQuery &query, bool includeSql = false);
QString errorText(const QSqlQuery *query, bool includeSql = false);
QString errorText(const QSqlTableModel &model);
QString errorText(const QSqlTableModel *model);
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


class ColDef
{
public:
    ColDef() {}
    ColDef(const QString& title, QVariant::Type type):
        _title(title), _type(type) { }

    //int index() const { return _index; }
    QString title() const { return _title; }
    QVariant::Type type() const { return _type; }

    QVariant value(const QSqlRecord& record) const;
    void setValue(QSqlRecord& record, const QVariant& value) const;

private:
    //int _index;
    QString _title;
    QVariant::Type _type;
};


class TableDef
{
public:
    int colCount() const { return _cols.size(); }
    const QString& tableName() const { return _tableName; }

    QString createTable() const;
    QString insertRecord(const QSqlRecord& record) const;

    QString sqlSelectAll(const QString& orderBy = QString()) const;

    const QVector<ColDef>& cols() const { return _cols; }

protected:
    TableDef(const QString& tableName): _tableName(tableName) {}
    QVector<ColDef> _cols;

private:
    QString _tableName;

    QString sqlColumns() const;
    QString sqlCreate() const;
};

#define DECLARE_COL(name, type)\
    const Ori::Sql::ColDef& name()\
    {\
        static int index = -1;\
        if (index < 0)\
        {\
            index = _cols.size();\
            _cols.append(Ori::Sql::ColDef(#name, type));\
        }\
        return _cols.at(index);\
    }

} // namespace Sql
} // namespace Ori

#endif // SQL_HELPERS_H
