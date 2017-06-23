#ifndef BUG_OPERATIONS_H
#define BUG_OPERATIONS_H

#include <QObject>

// TODO merge this into Operations
class BugOperations : public QObject
{
    Q_OBJECT

public:
    static BugOperations *instance();

    void raiseBugAdded(int id);
    void raiseBugChanged(int id);
    void raiseBugDeleted(int id);
    void raiseBugCommentAdded(int bugId);

signals:
    void bugAdded(int id);
    void bugChanged(int id);
    void bugDeleted(int id);
    void bugCommentAdded(int bugId);
};

#endif // BUG_OPERATIONS_H
