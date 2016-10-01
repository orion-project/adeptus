#include <QApplication>
#include <QBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QSqlField>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QToolBar>

#include "appearance.h"
#include "bugmanager.h"
#include "filterpanel.h"
#include "issuetable.h"
#include "helpers/OriWidgets.h"

//-----------------------------------------------------------------------------------------------

class IssueTableItemDelegate : public QStyledItemDelegate
{
public:
    IssueTableItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent)
    {
    }

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
    {
        QStyledItemDelegate::initStyleOption(option, index);
        QStyleOptionViewItemV4 *optionV4 = qstyleoption_cast<QStyleOptionViewItemV4*>(option);

        int row = index.row();
        int col = index.column();
        switch (index.sibling(row, COL_STATUS).data().toInt())
        {
        case STATUS_SOLVED:
            optionV4->backgroundBrush = ColorProvider::solvedColorTransparent();
            break;

        case STATUS_CLOSED:
            optionV4->backgroundBrush = ColorProvider::closedColorTransparent();
            break;

        case STATUS_OPENED:
            {
                int severity = index.sibling(row, COL_SEVERITY).data().toInt();
                switch (severity)
                {
                case SEVERITY_BLUNDER:
                    optionV4->backgroundBrush = QColor(255, 0, 0, 35);
                    break;
                case SEVERITY_CRUSH:
                    optionV4->backgroundBrush = QColor(255, 0, 0, 50);
                    break;
                case SEVERITY_BLOCKER:
                    optionV4->backgroundBrush = QColor(255, 0, 0, 75);
                    break;
                }
                if (severity <= SEVERITY_ENHANCE && col == COL_SEVERITY)
                        optionV4->font.setItalic(true);
                else if (severity >= SEVERITY_CRUSH && col == COL_SEVERITY)
                        optionV4->font.setBold(true);

                int priority = index.sibling(row, COL_PRIORITY).data().toInt();
                if (priority >= PRIORITY_HIGH && col == COL_PRIORITY)
                    optionV4->font.setBold(true);
            }
            break;
        }

        switch (index.column())
        {
        case COL_SUMMARY:
            optionV4->text = index.data().toString();
            optionV4->text.replace('\n', ' ');
            break;

        case COL_SOLUTION:
        case COL_STATUS:
        case COL_SEVERITY:
        case COL_PRIORITY:
        case COL_REPEAT:
        case COL_CATEGORY:
            optionV4->text = BugManager::displayDictValue(index.column(), index.data());
            break;

        case COL_CREATED:
        case COL_UPDATED:
            optionV4->text = BugManager::displayDateTime(index.data());
            break;
        }
    }
};

//-----------------------------------------------------------------------------------------------

IssueTableWidget::IssueTableWidget(QWidget *parent) : QWidget(parent)
{
    filterPanel = new FilterPanel;
    connect(filterPanel, SIGNAL(filterChanged()), this, SLOT(applyFilter()));

    auto toolbar = new QToolBar;
    toolbar->setIconSize(QSize(32, 32));
    toolbar->addAction(QIcon(":/tools/append"), tr("New issue..."), this, SLOT(appendBug()));

    auto toolbarLayout = new QHBoxLayout;
    toolbarLayout->setMargin(0);
    toolbarLayout->setSpacing(0);
    toolbarLayout->addWidget(toolbar);
    toolbarLayout->addWidget(filterPanel);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(3);
    mainLayout->setSpacing(3);
    mainLayout->addLayout(toolbarLayout);
}

IssueTableWidget::~IssueTableWidget()
{
    if (tableModel)
        delete tableModel;
}

QSqlTableModel* IssueTableWidget::update()
{
    createTableView();
    return tableModel;
}

void IssueTableWidget::close()
{
    if (tableView)
    {
        mainLayout->removeWidget(tableView);
        delete tableView;
        tableView = nullptr;
    }
    if (tableModel)
    {
        delete tableModel;
        tableModel = nullptr;
    }
}

void IssueTableWidget::createTableView()
{
    close();

    if (!itemDelegate)
        itemDelegate = new IssueTableItemDelegate(this);

    tableModel = new QSqlTableModel;
    tableModel->setTable(TABLE_BUGS);
    tableModel->setHeaderData(COL_ID, Qt::Horizontal, BugManager::columnTitle(COL_ID));
    tableModel->setHeaderData(COL_SUMMARY, Qt::Horizontal, BugManager::columnTitle(COL_SUMMARY));
    tableModel->setHeaderData(COL_EXTRA, Qt::Horizontal, BugManager::columnTitle(COL_EXTRA));
    tableModel->setHeaderData(COL_CATEGORY, Qt::Horizontal, BugManager::columnTitle(COL_CATEGORY));
    tableModel->setHeaderData(COL_SEVERITY, Qt::Horizontal, BugManager::columnTitle(COL_SEVERITY));
    tableModel->setHeaderData(COL_PRIORITY, Qt::Horizontal, BugManager::columnTitle(COL_PRIORITY));
    tableModel->setHeaderData(COL_REPEAT, Qt::Horizontal, BugManager::columnTitle(COL_REPEAT));
    tableModel->setHeaderData(COL_STATUS, Qt::Horizontal, BugManager::columnTitle(COL_STATUS));
    tableModel->setHeaderData(COL_SOLUTION, Qt::Horizontal, BugManager::columnTitle(COL_SOLUTION));
    tableModel->setHeaderData(COL_CREATED, Qt::Horizontal, BugManager::columnTitle(COL_CREATED));
    tableModel->setHeaderData(COL_UPDATED, Qt::Horizontal, BugManager::columnTitle(COL_UPDATED));
    _selected = false; // table is selected when filters are loaded

    tableView = new QTableView;
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSortingEnabled(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->verticalHeader()->hide();
    tableView->setAlternatingRowColors(true);
    tableView->setModel(tableModel);
    tableView->setColumnHidden(COL_EXTRA, true);
    tableView->setColumnHidden(COL_CREATED, true);
    tableView->setColumnHidden(COL_REPEAT, true);
    tableView->setItemDelegate(itemDelegate);

    Ori::Gui::adjustFont(tableView);
    adjustHeader();

    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(tableViewDoubleClicked(QModelIndex)));
    connect(tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableViewContextMenuRequested(QPoint)));

    mainLayout->addWidget(tableView);
    filterPanel->updateModels();
    tableView->setFocus();
}

void IssueTableWidget::adjustHeader()
{
    tableView->resizeColumnToContents(COL_ID);
    tableView->resizeColumnToContents(COL_CATEGORY);
    tableView->resizeColumnToContents(COL_SEVERITY);
    tableView->resizeColumnToContents(COL_PRIORITY);
    tableView->resizeColumnToContents(COL_STATUS);
    tableView->resizeColumnToContents(COL_SOLUTION);
    tableView->resizeColumnToContents(COL_UPDATED);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    tableView->horizontalHeader()->setSectionResizeMode(COL_SUMMARY, QHeaderView::Stretch);
#else
    tableView->horizontalHeader()->setResizeMode(COL_SUMMARY, QHeaderView::Stretch);
#endif
}

void IssueTableWidget::applyFilter()
{
    if (!tableModel) return;
    filterChanged = true;
    IssueFilters filters;
    filterPanel->collectFilters(&filters);
    applyFilters(&filters);
}

void IssueTableWidget::applyFilters(IssueFilters *filters)
{
    tableModel->setFilter(filters->getSql());
    if (!_selected)
    {
        _selected = true;
        tableModel->select();
    }
    adjustHeader();
    emit onFilter();
}

void IssueTableWidget::saveFilters()
{
    IssueFilters filters;
    filterPanel->collectFilters(&filters);
    QString res = filters.save();
    filterChanged = false;
    if (!res.isEmpty())
        QMessageBox::critical(this, qApp->applicationName(),
            tr("Unable to save filters.\n\n%1").arg(res));
}

void IssueTableWidget::loadFilters()
{
    filterPanel->load();
}

void IssueTableWidget::setSelectedRow(int row)
{
    if (!tableView) return;
    tableView->selectRow(row);
    tableView->setFocus();
    adjustHeader();
}

void IssueTableWidget::setSelectedId(int id)
{
    if (!tableModel or !tableView) return;
    QModelIndexList indexes = tableModel->match(
            tableModel->index(0, COL_ID), Qt::DisplayRole,
            QVariant::fromValue(id), 1, Qt::MatchExactly);
    if (!indexes.empty())
    {
        tableView->selectRow(indexes.at(0).row());
        tableView->scrollTo(indexes.at(0));
    }
}

int IssueTableWidget::selectedId()
{
    int row = selectedRow();
    if (row > -1)
    {
        return tableModel->record(row).field(COL_ID).value().toInt();
    }
    return -1;
}

int IssueTableWidget::selectedRow()
{
    if (tableModel)
    {
        QModelIndexList selection = tableView->selectionModel()->selectedRows();
        if (!selection.empty())
            return selection.at(0).row();
    }
    return -1;
}

void IssueTableWidget::tableViewDoubleClicked(const QModelIndex&)
{
    emit onDoubleClick();
}

void IssueTableWidget::tableViewContextMenuRequested(const QPoint& pos)
{
    if (contextMenu) contextMenu->exec(tableView->mapToGlobal(pos));
}

void IssueTableWidget::setFocus()
{
    if (tableView) tableView->setFocus();
}

void IssueTableWidget::appendBug()
{
    emit onAppendBug();
}
