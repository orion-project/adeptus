#include <QtSql>
#include <QApplication>
#include <QBoxLayout>
#include <QDataWidgetMapper>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPlainTextEdit>

#include "bugeditor.h"
#include "bugmanager.h"
#include "bugoperations.h"
#include "markdowneditor.h"
#include "preferences.h"
#include "bugitemdelegate.h"
#include "sqlbugprovider.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "tools/OriSettings.h"

using namespace Ori::Layouts;

#define PROP_SPACING    Space(8)

#define MODE_APPEND     0
#define MODE_EDIT       1

QMap<int, BugEditor*> __BugEditor_openedWindows;

class SummaryTextEdit : public QPlainTextEdit
{
public:
    QSize sizeHint() const { return QSize(400, 32); }
};

void BugEditor::append(QWidget *parent)
{
    BugEditor *wnd = new BugEditor(parent);
    wnd->setWindowTitle(qApp->tr("Append Issue"));
    wnd->initAppend();
    wnd->show();
    wnd->activateWindow();
}

void BugEditor::edit(QWidget *parent, int id)
{
    if (BugManager::isInvalid(id)) return;

    BugEditor *wnd;
    if (__BugEditor_openedWindows.contains(id))
    {
        wnd = __BugEditor_openedWindows[id];
    }
    else
    {
        wnd = new BugEditor(parent);
        wnd->setWindowTitle(qApp->tr("Edit Issue #%1").arg(id));
        QString res = wnd->initEdit(id);
        if (!res.isEmpty())
        {
            Ori::Dlg::error(res);
            delete wnd;
            return;
        }
        __BugEditor_openedWindows.insert(id, wnd);
        wnd->connect(BugOperations::instance(), SIGNAL(bugDeleted(int)), wnd, SLOT(bugDeleted(int)));
    }
    wnd->show();
    qApp->setActiveWindow(wnd);
}

BugEditor::BugEditor(QWidget *parent) : QWidget(parent)
{
    tableModel = new QSqlTableModel(this);
    tableModel->setTable(TABLE_BUGS);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool);
    setObjectName("BugEditor");

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(save()));

    comboCategory = WidgetHelper::createDictionaryCombo(COL_CATEGORY);
    comboStatus = WidgetHelper::createDictionaryCombo(COL_STATUS);
    comboSeverity = WidgetHelper::createDictionaryCombo(COL_SEVERITY);
    comboPriority = WidgetHelper::createDictionaryCombo(COL_PRIORITY);
    comboRepeat = WidgetHelper::createDictionaryCombo(COL_REPEAT);
    comboSolution = WidgetHelper::createDictionaryCombo(COL_SOLUTION);
    dateCreated = new QDateTimeEdit;
    dateUpdated = new QDateTimeEdit;
    textSummary = new SummaryTextEdit;
    textSummary->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    textExtra = new MarkdownEditor(BugManager::columnTitle(COL_EXTRA));

    if (!Preferences::instance().bugEditorEnableDates)
    {
        dateCreated->setEnabled(false);
        dateUpdated->setEnabled(false);
    }

    Ori::Gui::adjustFont(textSummary);

    LayoutH({
        LayoutV({
                            columnTitle(COL_CREATED), dateCreated, PROP_SPACING,
                            columnTitle(COL_UPDATED), dateUpdated, PROP_SPACING,
            labelStatus =   columnTitle(COL_STATUS), comboStatus, PROP_SPACING,
                            columnTitle(COL_SEVERITY), comboSeverity, PROP_SPACING,
                            columnTitle(COL_CATEGORY), comboCategory, PROP_SPACING,
                            columnTitle(COL_PRIORITY), comboPriority, PROP_SPACING,
            labelSolution = columnTitle(COL_SOLUTION), comboSolution, PROP_SPACING,
                            columnTitle(COL_REPEAT), comboRepeat,
                            Stretch()
        }),
        PROP_SPACING,
        LayoutV({
            columnTitle(COL_SUMMARY),
            textSummary,
            PROP_SPACING,
            textExtra,
            PROP_SPACING,
            buttons
        })
    })
        .useFor(this);

    Ori::Settings::restoreWindow(this, QSize(800, 480));

    textSummary->setFocus();
}

BugEditor::~BugEditor()
{
    Ori::Settings::storeWindow(this);
}

QLabel* BugEditor::columnTitle(int columnId)
{
    return new QLabel(BugManager::columnTitle(columnId));
}

void BugEditor::reject()
{
    if (Preferences::instance().confirmCancel && (
                textSummary->document()->isModified() ||
                textExtra->isModified()))
        if (!Ori::Dlg::yes(tr("Text has been changed. Cancel anyway?"))) return;

    close();
}

void BugEditor::initAppend()
{
    currentId = -1;

    connect(dateCreated, SIGNAL(dateTimeChanged(QDateTime)), dateUpdated, SLOT(setDateTime(QDateTime)));

    QDateTime now = QDateTime::currentDateTime();
    dateCreated->setDateTime(now);
    dateUpdated->setDateTime(now);
    WidgetHelper::selectId(comboCategory, CATEGORY_NONE);
    WidgetHelper::selectId(comboSeverity, SEVERITY_ERROR);
    WidgetHelper::selectId(comboPriority, PRIORITY_NORMAL);
    WidgetHelper::selectId(comboRepeat, REPEAT_ALWAYS);
    WidgetHelper::selectId(comboStatus, STATUS_OPENED);
    WidgetHelper::selectId(comboSolution, SOLUTION_NONE);

    labelStatus->setVisible(false);
    comboStatus->setVisible(false);
    labelSolution->setVisible(false);
    comboSolution->setVisible(false);

    mode = MODE_APPEND;
}

QString BugEditor::initEdit(int id)
{
    currentId = id;
    tableModel->setFilter(QString("Id = %1").arg(currentId));
    tableModel->select();

    if (tableModel->rowCount() != 1)
        return tr("Issue not found (#%1)").arg(id);

    mapper = new QDataWidgetMapper(this);
    mapper->setModel(tableModel);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setItemDelegate(new BugItemDelegate(this));
    mapper->addMapping(textSummary, COL_SUMMARY);
    mapper->addMapping(textExtra->editor(), COL_EXTRA);
    mapper->addMapping(comboCategory, COL_CATEGORY);
    mapper->addMapping(comboSeverity, COL_SEVERITY);
    mapper->addMapping(comboPriority, COL_PRIORITY);
    mapper->addMapping(comboRepeat, COL_REPEAT);
    mapper->addMapping(comboStatus, COL_STATUS);
    mapper->addMapping(comboSolution, COL_SOLUTION);
    mapper->addMapping(dateCreated, COL_CREATED);
    mapper->addMapping(dateUpdated, COL_UPDATED);
    mapper->setCurrentModelIndex(tableModel->index(0, 0));

    mode = MODE_EDIT;
    return QString();
}

void BugEditor::save()
{
    if (textSummary->toPlainText().trimmed().isEmpty())
    {
        Ori::Dlg::warning(tr("Summary text must not be empty."));
        return;
    }

    QString result;

    switch (mode)
    {
    case MODE_APPEND:
        result = saveNew();
        break;

    case MODE_EDIT:
        result = saveEdit();
        break;
    }

    if (!result.isEmpty())
    {
        Ori::Dlg::error(tr("Unable to save issue.\n\n%1").arg(result));
        return;
    }

    switch (mode)
    {
    case MODE_APPEND:
        BugOperations::instance()->raiseBugAdded(currentId);
        break;

    case MODE_EDIT:
        BugOperations::instance()->raiseBugChanged(currentId);
        break;
    }

    close();
}

QString BugEditor::saveNew()
{
    QVariant id = BugManager::generateBugId();
    if (id.type() == QVariant::String)
        return tr("Unable to generate new issue id.\n\n%1").arg(id.toString());

    QSqlRecord record;
    SqlHelper::addField(record, "Id", QVariant::Int, id);
    SqlHelper::addField(record, "Summary", QVariant::String, textSummary->toPlainText().trimmed());
    SqlHelper::addField(record, "Extra", QVariant::String, textExtra->getText());
    SqlHelper::addField(record, "Category", QVariant::Int, WidgetHelper::selectedId(comboCategory));
    SqlHelper::addField(record, "Severity", QVariant::Int, WidgetHelper::selectedId(comboSeverity));
    SqlHelper::addField(record, "Priority", QVariant::Int, WidgetHelper::selectedId(comboPriority));
    SqlHelper::addField(record, "Repeat", QVariant::Int, WidgetHelper::selectedId(comboRepeat));
    SqlHelper::addField(record, "Status", QVariant::Int, WidgetHelper::selectedId(comboStatus));
    SqlHelper::addField(record, "Solution", QVariant::Int, WidgetHelper::selectedId(comboSolution));
    SqlHelper::addField(record, "Created", QVariant::DateTime, dateCreated->dateTime());
    SqlHelper::addField(record, "Updated", QVariant::DateTime, dateUpdated->dateTime());

    if (!tableModel->insertRecord(-1, record))
        return SqlHelper::errorText(tableModel);

    this->currentId = id.toInt();

    return QString();
}

QString BugEditor::saveEdit()
{
    if (!Preferences::instance().bugEditorEnableDates)
        dateUpdated->setDateTime(QDateTime::currentDateTime());

    IssueInfo oldValues = tableIssues().recordToObject(tableModel->record(0));

    if (!mapper->submit())
        return SqlHelper::errorText(tableModel);

    IssueInfo newValues = tableIssues().recordToObject(tableModel->record(0));
    QString res = BugComparer::writeHistory(oldValues, newValues);
    if (!res.isEmpty()) // do not return error, only show it
        Ori::Dlg::error(tr("Error while writing history:\n\n%1").arg(res));

    return QString();
}

void BugEditor::bugDeleted(int id)
{
    if (id == currentId) close();
}
