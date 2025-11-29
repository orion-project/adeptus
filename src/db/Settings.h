#ifndef DB_SETTINGS_H
#define DB_SETTINGS_H

#include <QVariant>

namespace Db {

class Settings
{
public:
    Settings(bool transaction);
    ~Settings();
    
    QString set(const QString& name, const QVariant& value);
    QString get(const QString& name, QVariant& value, const QVariant &def = QVariant());
    
    QString lastError;
    
private:
    bool _transaction;

    void rollback();
    QString lastErrorStr();
};

} // namespace Db

#endif // DB_SETTINGS_H