#ifndef BUGITEMDELEGATE_H
#define BUGITEMDELEGATE_H

#include <QItemDelegate>

class BugItemDelegate : public QItemDelegate
{
public:
    BugItemDelegate(QObject *parent = 0);
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // BUGITEMDELEGATE_H
