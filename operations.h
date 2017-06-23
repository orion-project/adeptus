#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <QObject>

class Operations : public QObject
{
    Q_OBJECT

public:
    static Operations* instance();

    static void showIssue(int id);
    static void deleteIssue(int id);
    static void commentIssue(int id);

    static void makeRelation(int id);
    static void deleteRelation(int id1, int id2);

signals:
    void requestShowIssue(int id);

    void issueDeleted(int id);
    void issueChanged(int id);

private:
    explicit Operations(QObject *parent = 0) : QObject(parent) {}
};

#endif // OPERATIONS_H
