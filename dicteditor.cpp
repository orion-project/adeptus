#include <QtSql>
#include <QAction>
#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QDataWidgetMapper>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSpinBox>
#include <QTableView>
#include <limits.h>

#include "dicteditor.h"
#include "bugmanager.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"

bool DictEditor::show(QWidget *parent, const QString &title, int dictId)
{
    if (!QSqlDatabase::contains()) return false;

    DictEditor *editor = new DictEditor(parent, dictId);
    editor->setWindowTitle(title);
    editor->exec();
    return editor->modified;
}

DictEditor::DictEditor(QWidget *parent, int dictId) : QDialog(parent), modified(false)
{
    setAttribute(Qt::WA_DeleteOnClose);

    tableModel = BugManager::dictionary(dictId);
    tableModel->setHeaderData(DICT_COL_ID, Qt::Horizontal, qApp->translate("Dict editor column", "ID"));
    tableModel->setHeaderData(DICT_COL_TITLE, Qt::Horizontal, qApp->translate("Dict editor column", "Value"));

    tableView = new QTableView;
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSortingEnabled(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setShowGrid(false);
    tableView->verticalHeader()->hide();
    tableView->setAlternatingRowColors(true);
    tableView->setModel(tableModel);
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editItem()));

    Ori::Gui::adjustFont(tableView);
    adjustHeader();

    createActions();

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setMargin(6);
    layoutMain->addWidget(tableView);

    setLayout(layoutMain);

    resize(300, 400);
}

void DictEditor::createActions()
{
    actionEdit = new QAction(tr("&Edit..."), this);
    connect(actionEdit, SIGNAL(triggered()), this, SLOT(editItem()));

    actionDelete = new QAction(tr("&Delete"), this);
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(deleteItem()));

    actionAppend = new QAction(tr("&Append..."), this);
    connect(actionAppend, SIGNAL(triggered()), this, SLOT(appendItem()));
}

void DictEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(actionEdit);
    menu.addAction(actionDelete);
    menu.addSeparator();
    menu.addAction(actionAppend);
    menu.exec(event->globalPos());
}

void DictEditor::deleteItem()
{
    QModelIndexList selection = tableView->selectionModel()->selectedRows();
    if (!selection.empty())
    {
        QModelIndex index = selection.first();
        int row = index.row();
        int id = index.sibling(row, DICT_COL_ID).data().toInt();
        QString title = index.sibling(row, DICT_COL_TITLE).data().toString();
        if (Ori::Dlg::yes(tr("Delete element #%1 \"%2\"?").arg(id).arg(title)))
        {
            if (!tableModel->removeRow(row))
            {
                Ori::Dlg::error(tr("Unable to delete element:\n\n%1").arg(tableModel->lastError().text()));
                tableModel->select();
                return;
            }
            tableModel->select();
            modified = true;
            adjustHeader();
        }
    }
}

void DictEditor::editItem()
{
    QModelIndexList selection = tableView->selectionModel()->selectedRows();
    if (!selection.empty())
    {
        QModelIndex selectedIndex = selection.at(0);
        if (editElement(&selectedIndex))
        {
            tableView->selectRow(selectedIndex.row());
            tableView->setFocus();
            adjustHeader();
            modified = true;
        }
    }
}

void DictEditor::appendItem()
{
    if (editElement(NULL))
    {
        tableView->selectRow(tableModel->rowCount()-1);
        tableView->setFocus();
        adjustHeader();
        modified = true;
    }
}

void DictEditor::adjustHeader()
{
    tableView->resizeColumnToContents(0);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
#else
    tableView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
#endif
}

bool DictEditor::editElement(QModelIndex *item)
{
    QDialog dlg(this);

    QSpinBox *id = new QSpinBox;
    id->setFont(tableView->font());
    id->setRange(INT_MIN, INT_MAX);

    QLineEdit *text = new QLineEdit;
    text->setFont(tableView->font());

    QDataWidgetMapper *mapper = NULL;
    if (item) // edit mode
    {
        setWindowTitle(tr("Edit Element"));
        mapper = new QDataWidgetMapper(this);
        mapper->setModel(tableModel);
        mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
        mapper->addMapping(id, DICT_COL_ID);
        mapper->addMapping(text, DICT_COL_TITLE);
        mapper->setCurrentModelIndex(*item);
    }
    else
        setWindowTitle(tr("Append Element"));

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    connect(buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &dlg, SLOT(reject()));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tableModel->headerData(DICT_COL_ID, Qt::Horizontal).toString(), id);
    layout->addRow(tableModel->headerData(DICT_COL_TITLE, Qt::Horizontal).toString(), text);
    layout->addItem(new QSpacerItem(0, 6));
    layout->addRow(buttons);

    dlg.setLayout(layout);
    dlg.setSizeGripEnabled(true);
    if (dlg.exec())
    {
        if (item && mapper)
        {
            if (!mapper->submit())
            {
                Ori::Dlg::error(tr("Unable to save changed element:\n\n%1").arg(tableModel->lastError().text()));
                tableModel->select();
                return false;
            }
        }
        else
        {
            QSqlRecord r;
            SqlHelper::addField(r, DICT_ID, id->value());
            SqlHelper::addField(r, DICT_TITLE, text->text().trimmed());
            if (!tableModel->insertRecord(-1, r))
            {
                Ori::Dlg::error(tr("Unable to create new element:\n\n%1").arg(tableModel->lastError().text()));
                tableModel->select();
                return false;
            }
        }
        return true;
    }
    return false;
}

