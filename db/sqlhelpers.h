#ifndef SQL_HELPERS_H
#define SQL_HELPERS_H

#include <QtSql>
#include <QString>
#include <QDebug>

//-----------------------------------------------------------------------------------------------

namespace SqlHelper {

void addField(QSqlRecord &record, const QString &name, QMetaType::Type type, const QVariant &value);
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

class ActionQuery
{
public:    
    ActionQuery(const QString& sql)
    {
        _query.prepare(sql);
    }

    ActionQuery& param(const QString& name, const QVariant& value)
    {
        _query.bindValue(name, value);
        return *this;
    }

    QString exec()
    {
        if (!_query.exec())
            return SqlHelper::errorText(_query, true);
        return QString();
    }
    
private:
    QSqlQuery _query;
};


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
    ColDef(const QString& title, QMetaType type):
        _title(title), _type(type) { }

    QString title() const { return _title; }
    QMetaType type() const { return _type; }

    QVariant fromRecord(const QSqlRecord& record) const;
    void toRecord(QSqlRecord& record, const QVariant& value) const;

private:
    QString _title;
    QMetaType _type;
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

} // namespace Sql
} // namespace Ori

#endif // SQL_HELPERS_H
