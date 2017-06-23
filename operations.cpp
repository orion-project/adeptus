#include "operations.h"
#include "bugsolver.h"
#include "db/db.h"
#include "helpers/OriDialogs.h"

#include <QApplication>
#include <QInputDialog>

Operations* Operations::instance() { static Operations instance; return &instance; }

void Operations::showIssue(int id)
{
    emit instance()->requestShowIssue(id);
}

void Operations::deleteIssue(int id)
{
    if (id > 0 && Ori::Dlg::yes(tr("Delete issue #%1?").arg(id)))
    {
        auto res = DB::issues().remove(id);
        if (!res.isEmpty())
        {
            Ori::Dlg::error(tr("Failed to delete issue #%1:\n\n%2").arg(id).arg(res));
            return;
        }
        emit instance()->issueDeleted(id);
    }
}

void Operations::commentIssue(int id)
{
    BugSolver::comment(qApp->activeWindow(), id);
}

void Operations::makeRelation(int id)
{
    int id1 = id;
    if (id1 < 0) return;
    bool ok;
    int id2 = QInputDialog::getInt(qApp->activeWindow(), qApp->tr("Make Relation"),
        qApp->tr("Make relation for #%1.\nRelated issue identifier:").arg(id1), 1, 1, INT_MAX, 1, &ok);
    if (!ok) return;
    QString  res = DB::relations().make(id1, id2);
    if (!res.isEmpty())
        Ori::Dlg::error(res);
    emit instance()->issueChanged(id1);
    emit instance()->issueChanged(id2);
}

void Operations::deleteRelation(int id1, int id2)
{
    if (Ori::Dlg::yes(qApp->tr("Delete relation [#%1 - #%2]?").arg(id1).arg(id2)))
    {
        QString res = DB::relations().remove(id1, id2);
        if (!res.isEmpty())
            Ori::Dlg::error(res);
        emit instance()->issueChanged(id1);
        emit instance()->issueChanged(id2);
    }
}
