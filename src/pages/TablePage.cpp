#include "TablePage.h"

#include "../bugmanager.h"
#include "../filterpanel.h"
#include "../db/Dicts.h"

#include "helpers/OriWidgets.h"

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

        int row = index.row();
        int col = index.column();
        
        QColor backColor, textColor;
        bool fontB = false;
        bool fontI = false;
        bool fontU = false;
        bool fontS = false;
        for (int dictId : Db::Dicts::dictIdsForStyling())
        {
            int valId = index.sibling(row, dictId).data().toInt();
            auto style = Db::Dicts::style(dictId, valId);
            if (!style)
                continue;
            if (style->rowBackColor) backColor = *style->rowBackColor;
            if (style->rowTextColor) textColor = *style->rowTextColor;
            if (style->rowFontB) fontB = *style->rowFontB;
            if (style->rowFontI) fontI = *style->rowFontI;
            if (style->rowFontU) fontI = *style->rowFontU;
            if (style->rowFontS) fontI = *style->rowFontS;
            if (dictId == col)
            {
                if (style->cellBackColor) backColor = *style->cellBackColor;
                if (style->cellTextColor) textColor = *style->cellTextColor;
                if (style->cellFontB) fontB = *style->cellFontB;
                if (style->cellFontI) fontI = *style->cellFontI;
                if (style->cellFontU) fontI = *style->cellFontU;
                if (style->cellFontS) fontI = *style->cellFontS;
            }
        }
        
        if (backColor.isValid()) option->backgroundBrush = backColor;
        if (textColor.isValid()) option->palette.setBrush(QPalette::Text, textColor);
        if (fontB) option->font.setBold(true);
        if (fontI) option->font.setItalic(true);
        if (fontU) option->font.setUnderline(true);
        if (fontS) option->font.setStrikeOut(true);

        switch (index.column())
        {
        case COL_SUMMARY:
            option->text = index.data().toString();
            option->text.replace('\n', ' ');
            break;

        case COL_SOLUTION:
        case COL_STATUS:
        case COL_SEVERITY:
        case COL_PRIORITY:
        case COL_REPEAT:
        case COL_CATEGORY:
            option->text = Db::Dicts::value(index.column(), index.data());
            break;

        case COL_CREATED:
        case COL_UPDATED:
            option->text = BugManager::displayDateTime(index.data());
            break;
        }
    }
};

//-----------------------------------------------------------------------------------------------

TablePage::TablePage(QWidget *parent) : QWidget(parent)
{
    filterPanel = new FilterPanel;
    connect(filterPanel, SIGNAL(filterChanged()), this, SLOT(applyFilter()));

    auto toolbar = new QToolBar;
    toolbar->setIconSize(QSize(32, 32));
    toolbar->addAction(QIcon(":/tools/append"), tr("New issue..."), this, SLOT(appendBug()));

    auto toolbarLayout = new QHBoxLayout;
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(0);
    toolbarLayout->addWidget(toolbar);
    toolbarLayout->addWidget(filterPanel);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(3, 3, 3, 3);
    mainLayout->setSpacing(3);
    mainLayout->addLayout(toolbarLayout);
}

TablePage::~TablePage()
{
    if (tableModel)
        delete tableModel;
}

QSqlTableModel* TablePage::update()
{
    createTableView();
    return tableModel;
}

void TablePage::close()
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

void TablePage::createTableView()
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
    //tableView->setAlternatingRowColors(true);
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

void TablePage::adjustHeader()
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

void TablePage::applyFilter()
{
    if (!tableModel) return;
    filterChanged = true;
    IssueFilters filters;
    filterPanel->collectFilters(&filters);
    applyFilters(&filters);
}

void TablePage::applyFilters(IssueFilters *filters)
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

void TablePage::saveFilters()
{
    IssueFilters filters;
    filterPanel->collectFilters(&filters);
    QString res = filters.save();
    filterChanged = false;
    if (!res.isEmpty())
        QMessageBox::critical(this, qApp->applicationName(),
            tr("Unable to save filters.\n\n%1").arg(res));
}

void TablePage::loadFilters()
{
    filterPanel->load();
}

void TablePage::setSelectedRow(int row)
{
    if (!tableView) return;
    tableView->selectRow(row);
    tableView->setFocus();
    adjustHeader();
}

void TablePage::setSelectedId(int id)
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

int TablePage::selectedId()
{
    int row = selectedRow();
    if (row > -1)
    {
        return tableModel->record(row).field(COL_ID).value().toInt();
    }
    return -1;
}

int TablePage::selectedRow()
{
    if (tableModel)
    {
        QModelIndexList selection = tableView->selectionModel()->selectedRows();
        if (!selection.empty())
            return selection.at(0).row();
    }
    return -1;
}

void TablePage::tableViewDoubleClicked(const QModelIndex&)
{
    emit onDoubleClick();
}

void TablePage::tableViewContextMenuRequested(const QPoint& pos)
{
    if (contextMenu) contextMenu->exec(tableView->mapToGlobal(pos));
}

void TablePage::setFocus()
{
    if (tableView) tableView->setFocus();
}

void TablePage::appendBug()
{
    emit onAppendBug();
}
