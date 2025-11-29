#include "Settings.h"

#include "Db.h"

namespace Db {

Settings::Settings(bool transaction): _transaction(transaction)
{
    if (transaction)
        if (!db()->transaction())
            lastError = SqlHelper::errorText(db()->lastError());
}

Settings::~Settings()
{
    if (_transaction)
        db()->commit();
}

void Settings::rollback()
{
    if (_transaction)
    {
        lastError.clear();
        _transaction = false;
        if (!db()->rollback())
            lastError = SqlHelper::errorText(db()->lastError());
    }
}

QString Settings::lastErrorStr()
{
    return lastError.isEmpty() ? QString() : ("\n\n" + lastError);
}

QString Settings::set(const QString& name, const QVariant& value)
{
    QSqlQuery query;
    if (!query.exec(QString("select count(Name) from %1 where Name = '%2'").arg(TABLE_SETTINGS, name)))
    {
        rollback();
        return qApp->tr("Unable to save setting '%1': %2%3")
                .arg(name, SqlHelper::errorText(query), lastErrorStr());
    }
    QString sql;
    if (query.isSelect() && query.first() && query.record().value(0).toInt() == 0)
    {
        sql = QString("insert into %1 (Name, Value) values ('%2', '%3')")
                .arg(TABLE_SETTINGS, name, value.toString());
    }
    else
    {
        sql = QString("update %1 set Value = '%3' where Name = '%2'")
                .arg(TABLE_SETTINGS, name, value.toString());
    }
    if (!query.exec(sql))
    {
        rollback();
        return qApp->tr("Unable to save setting '%1': %2%3")
                .arg(name, SqlHelper::errorText(query), lastErrorStr());
    }
    return QString();
}

QString Settings::get(const QString& name, QVariant& value, const QVariant& def)
{
    QSqlQuery query;
    if (!query.exec(QString("select Value from %1 where Name = '%2'").arg(TABLE_SETTINGS, name)))
        return qApp->tr("Unable to load setting '%1': %2").arg(name, SqlHelper::errorText(query));
    if (query.isSelect() && query.first())
    {
        value = query.record().value(0);
        if (value.typeId() != QMetaType::QString)
            return qApp->tr("Setting '%1' has no proper type").arg(name);
        if (def.isValid() && def.typeId() != QMetaType::QString)
            if (!value.convert(def.metaType()))
                return qApp->tr("Setting '%1' has no proper type").arg(name);
    }
    else value = def;
    return QString();
}

} // namespace Db