#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <QObject>

class Operations : public QObject
{
    Q_OBJECT

public:
    enum Options {
        ShowIssue,
        RefreshIssue,
        CommentIssue
    };

    static Operations* instance();

    static void showIssue(int id);
    static void commentIssue(int id);
    static void makeRelation(int id);
    static void deleteRelation(int id1, int id2);

signals:
    void operationRequest(int operation, int id);

private:
    explicit Operations(QObject *parent = 0) : QObject(parent) {}

    static void requestRefresh(int id);
};

#endif // OPERATIONS_H
