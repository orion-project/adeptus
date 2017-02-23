#include <QtSql>
#include <QApplication>
#include <QBoxLayout>
#include <QDataWidgetMapper>
#include <QDialogButtonBox>
#include <QDateTimeEdit>
#include <QLabel>

#include "bugsolver.h"
#include "bugmanager.h"
#include "bugoperations.h"
#include "markdowneditor.h"
#include "preferences.h"
#include "bugitemdelegate.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"

QMap<int, BugSolver*> __BugSolver_openedWindows;

BugSolver* BugSolver::initWindow(QWidget *parent, int id, const QString& title)
{
    if (BugManager::isInvalid(id)) return nullptr;

    BugSolver *wnd;
    if (__BugSolver_openedWindows.contains(id))
    {
        wnd = __BugSolver_openedWindows[id];
    }
    else
    {
        wnd = new BugSolver(parent);
        auto res = wnd->initWindow(id);
        if (!res.isEmpty())
        {
            Ori::Dlg::error(res);
            delete wnd;
            return nullptr;
        }
        __BugSolver_openedWindows.insert(id, wnd);
    }
    wnd->connect(BugOperations::instance(), SIGNAL(bugDeleted(int)), wnd, SLOT(bugDeleted(int)));
    wnd->setWindowTitle(tr("%1 #%2").arg(title).arg(id));
    wnd->show();
    qApp->setActiveWindow(wnd);
    return wnd;
}

void BugSolver::process(QWidget *parent, int id, int currentStatus)
{
    switch (currentStatus)
    {
    case STATUS_OPENED: solve(parent, id); break;
    case STATUS_SOLVED: close(parent, id); break;
    case STATUS_CLOSED: reopen(parent, id); break;
    }
}

void BugSolver::solve(QWidget *parent, int id)
{
    BugSolver *wnd = initWindow(parent, id, qApp->tr("Solve Issue"));
    if (!wnd) return;

    WidgetHelper::selectId(wnd->comboStatus, STATUS_SOLVED);
    WidgetHelper::selectId(wnd->comboSolution, SOLUTION_FIXED);
    wnd->setIcon("solve");
}

void BugSolver::close(QWidget *parent, int id)
{
    BugSolver *wnd = initWindow(parent, id, qApp->tr("Close Issue"));
    if (!wnd) return;

    WidgetHelper::selectId(wnd->comboStatus, STATUS_CLOSED);
    wnd->comboSolution->setEnabled(false);
    wnd->setIcon("close");
}

void BugSolver::reopen(QWidget *parent, int id)
{
    BugSolver *wnd = initWindow(parent, id, qApp->tr("Reopen Issue"));
    if (!wnd) return;

    WidgetHelper::selectId(wnd->comboStatus, STATUS_OPENED);
    WidgetHelper::selectId(wnd->comboSolution, SOLUTION_NONE);
    wnd->setIcon("reopen");
}

void BugSolver::comment(QWidget *parent, int id)
{
    BugSolver *wnd = initWindow(parent, id, qApp->tr("Comment Issue"));
    if (!wnd) return;

    wnd->comboSolution->setEnabled(false);
    wnd->setIcon("comment");
}


QString BugSolver::initWindow(int id)
{
    currentId = id;
    tableModel = new QSqlTableModel(this);
    tableModel->setTable(TABLE_BUGS);
    tableModel->setFilter(QString("Id = %1").arg(currentId));
    tableModel->select();

    if (tableModel->rowCount() != 1)
        return QString("Issue not found (#%1)").arg(id);

    QSqlRecord record = tableModel->record(0);
    oldStatus = record.field("Status").value();
    oldSolution = record.field("Solution").value();

    mapper = new QDataWidgetMapper(this);
    mapper->setModel(tableModel);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setItemDelegate(new BugItemDelegate(this));
    mapper->addMapping(comboStatus, COL_STATUS);
    mapper->addMapping(comboSolution, COL_SOLUTION);
    mapper->addMapping(dateEvent, COL_UPDATED);
    mapper->setCurrentModelIndex(tableModel->index(0, 0));

    dateEvent->setDateTime(QDateTime::currentDateTime());

    return QString();
}

QByteArray __BugSolver_storedGeometry;

BugSolver::BugSolver(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool);

    textComment = new MarkdownEditor(tr("Comment"));

    comboStatus = WidgetHelper::createDictionaryCombo(COL_STATUS);
    comboStatus->setEnabled(false);

    comboSolution = WidgetHelper::createDictionaryCombo(COL_SOLUTION);

    dateEvent = new QDateTimeEdit;
    if (!Preferences::instance().bugSolverEnableDates)
        dateEvent->setEnabled(false);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(save()));

    QHBoxLayout *layoutProps = new QHBoxLayout;
    layoutProps->setSpacing(6);
    layoutProps->addWidget(new QLabel(tr("Date:")));
    layoutProps->addWidget(dateEvent);
    layoutProps->addSpacing(12);
    layoutProps->addWidget(new QLabel(BugManager::columnTitle(COL_STATUS) + ":"));
    layoutProps->addWidget(comboStatus);
    layoutProps->addSpacing(12);
    layoutProps->addWidget(new QLabel(BugManager::columnTitle(COL_SOLUTION) + ":"));
    layoutProps->addWidget(comboSolution);
    layoutProps->addStretch();
    layoutProps->addWidget(_labelIcon = new QLabel);

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setSpacing(3);
    layoutMain->addLayout(layoutProps);
    layoutMain->addSpacing(6);
    //layoutMain->addWidget(new QLabel(tr("Comment")));
    layoutMain->addWidget(textComment);
    layoutMain->addSpacing(12);
    layoutMain->addWidget(buttons);
    setLayout(layoutMain);

    if (!__BugSolver_storedGeometry.isEmpty())
        restoreGeometry(__BugSolver_storedGeometry);
    else resize(600, 300);

    textComment->setFocus();
}

BugSolver::~BugSolver()
{
    __BugSolver_storedGeometry = saveGeometry();
    __BugSolver_openedWindows.remove(currentId);
}

void BugSolver::setIcon(const QString& path)
{
    _labelIcon->setPixmap(QIcon(":/tools/" + path).pixmap(32, 32));
}

void BugSolver::reject()
{
    if (textComment->isModified() && Preferences::instance().confirmCancel)
        if (!Ori::Dlg::yes(tr("Text has been changed. Cancel anyway?"))) return;

    QWidget::close();
}

void BugSolver::save()
{
    QSqlDatabase db = tableModel->database();
    db.transaction();

    QDateTime moment = dateEvent->dateTime();
    QString comment = textComment->getText();
    QVariant newStatus = WidgetHelper::selectedId(comboStatus);
    QVariant newSolution = WidgetHelper::selectedId(comboSolution);
    QString res;

    QVariant eventNum = BugManager::generateEventId(currentId);
    if (eventNum.type() == QVariant::String)
    {
        db.rollback();
        Ori::Dlg::error(tr("Unable to generate history item number.\n\n%1").arg(eventNum.toString()));
        return;
    }

    if (comboSolution->isEnabled()) // solution is not changed when bug is closed
    {
        res = BugManager::addHistroyItem(currentId, eventNum, moment, COL_SOLUTION, oldSolution, newSolution);
        if (!res.isEmpty())
        {
            db.rollback();
            Ori::Dlg::error(tr("Unable to add issue history item (change solution).\n\n%1").arg(res));
            return;
        }
    }

    if (oldStatus.toInt() != newStatus.toInt()) // status is not changed when commenting, but it is changed in other cases
    {
        res = BugManager::addHistroyItem(currentId, eventNum, moment, COL_STATUS, oldStatus, newStatus);
        if (!res.isEmpty())
        {
            db.rollback();
            Ori::Dlg::error(tr("Unable to add issue history item (change status).\n\n%1").arg(res));
            return;
        }
    }

    if (!comment.isEmpty())
    {
        res = BugManager::addHistroyComment(currentId, eventNum, moment, comment);
        if (!res.isEmpty())
        {
            db.rollback();
            Ori::Dlg::error(tr("Unable to add comment to issue.\n\n%1").arg(res));
            return;
        }
    }

    res = saveIssue();
    if (!res.isEmpty())
    {
        db.rollback();
        Ori::Dlg::error(tr("Unable to save issue.\n\n%1").arg(res));
        return;
    }

    db.commit();

    BugOperations::instance()->raiseBugChanged(currentId);
    BugOperations::instance()->raiseBugCommentAdded(currentId);
    QWidget::close();
}

QString BugSolver::saveIssue()
{
    if (!mapper->submit())
        return SqlHelper::errorText(tableModel);

    return QString();
}

void BugSolver::bugDeleted(int id)
{
    if (id == currentId) QWidget::close();
}
