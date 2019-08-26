#ifndef BUG_ITEM_DELEGATE_H
#define BUG_ITEM_DELEGATE_H

#include <QItemDelegate>

class BugItemDelegate : public QItemDelegate
{
public:
    BugItemDelegate(QObject *parent = nullptr);
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // BUG_ITEM_DELEGATE_H
