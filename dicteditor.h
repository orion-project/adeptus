#ifndef DICTEDITOR_H
#define DICTEDITOR_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QModelIndex;
class QSqlTableModel;
class QTableView;
QT_END_NAMESPACE

class DictEditor : public QDialog
{
    Q_OBJECT

public:
    bool modified;

    explicit DictEditor(QWidget *parent, int dictId);
    static bool show(QWidget *parent, const QString &title, int dictId);

protected:
    void contextMenuEvent(QContextMenuEvent*);

private slots:
    void appendItem();
    void deleteItem();
    void editItem();

private:
    QSqlTableModel *tableModel;
    QTableView *tableView;
    QAction *actionAppend;
    QAction *actionDelete;
    QAction *actionEdit;

    void createActions();
    void adjustHeader();
    bool editElement(QModelIndex*);
};

#endif // DICTEDITOR_H
