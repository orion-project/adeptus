#include "sqlhelpers.h"

namespace SqlHelper {

void addField(QSqlRecord &record, const QString &name, QMetaType::Type type, const QVariant &value)
{
    QSqlField field(name, QMetaType(type));
    field.setValue(value);
    record.append(field);
}

void addField(QSqlRecord &record, const QString &name, const QVariant &value)
{
    QSqlField field(name, value.metaType());
    field.setValue(value);
    record.append(field);
}

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

QString errorText(const QSqlTableModel &model)
{
    return errorText(model.lastError());
}

QString errorText(const QSqlTableModel *model)
{
    return errorText(model->lastError());
}

QString errorText(const QSqlError &error)
{
    return QString("%1\n%2").arg(error.driverText()).arg(error.databaseText());
}

} // namespace SqlHelper


namespace Ori {
namespace Sql {

QVariant ColDef::fromRecord(const QSqlRecord& record) const
{
    return record.value(_title);
}

void ColDef::toRecord(QSqlRecord& record, const QVariant& value) const
{
    QSqlField field(_title, _type);
    field.setValue(value);
    record.append(field);
}

//-----------------------------------------------------------------------------

QString TableDef::createTable() const
{
    QSqlQuery query;
    if (!query.exec(sqlCreate()))
        return QString("Unable to create table %1.\n\n%2")
                .arg(tableName().toUpper())
                .arg(SqlHelper::errorText(query));
    return QString();
}

QString TableDef::insertRecord(const QSqlRecord& record) const
{
    QSqlTableModel model;
    model.setTable(tableName());

    return !model.insertRecord(-1, record)? SqlHelper::errorText(model): QString();
}

QString TableDef::sqlColumns() const
{
    QStringList cols;
    for (const ColDef& col: _cols)
        cols << col.title();
    return cols.join(",");
}


QString TableDef::sqlCreate() const
{
    return QString("create table if not exists %1 (%2)").arg(tableName()).arg(sqlColumns());
}

QString TableDef::sqlSelectAll(const QString& orderBy) const
{
    QString sql = QString("select %1 from %2").arg(sqlColumns()).arg(tableName());
    if (!orderBy.isEmpty()) sql += " order by " + orderBy;
    return sql;
}

} // namespace Sql
} // namespace Ori
