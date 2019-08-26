#ifndef ISSUE_TABLE_H
#define ISSUE_TABLE_H

#include <QWidget>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
class QMenu;
class QSqlTableModel;
class QStyledItemDelegate;
class QTableModel;
class QTableView;
class QVBoxLayout;
QT_END_NAMESPACE

class IssueFilters;
class FilterPanel;

//-----------------------------------------------------------------------------------------------

class IssueTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IssueTableWidget(QWidget *parent = nullptr);
    ~IssueTableWidget();

    QMenu* contextMenu = nullptr;

    QSqlTableModel* update();
    void setSelectedRow(int row);
    void setSelectedId(int id);
    int selectedRow();
    int selectedId();
    void adjustHeader();
    void saveFilters();
    void loadFilters();
    bool isFilterChanged() { return filterChanged; }
    void setFocus();
    void close();

signals:
    void onFilter();
    void onDoubleClick();
    void onAppendBug();

private:
    QSqlTableModel *tableModel = nullptr;
    QStyledItemDelegate *itemDelegate = nullptr;
    QTableView *tableView = nullptr;
    FilterPanel *filterPanel;
    QVBoxLayout *mainLayout;

    bool filterChanged = false;
    bool _selected;

    void createTableView();
    void applyFilters(IssueFilters *filters);

private slots:
    void applyFilter();
    void appendBug();
    void tableViewDoubleClicked(const QModelIndex&);
    void tableViewContextMenuRequested(const QPoint&);
};

#endif // ISSUE_TABLE_H
