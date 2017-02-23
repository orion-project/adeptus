#ifndef GUIACTIONS_H
#define GUIACTIONS_H

#include <QObject>

// TODO check if could be merged with BugOperations
class GuiActions : public QObject
{
    Q_OBJECT

public:
   static GuiActions* instance();

   static void showIssue(int id);
   static void deleteRelation(int id1, int id2);

signals:
   void operationRequest(int operation, int id);

private:
    explicit GuiActions(QObject *parent = 0);
};

#endif // GUIACTIONS_H
