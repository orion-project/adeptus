#include <QApplication>

#include "bugmanager.h"
#include "guiactions.h"
#include "helpers/OriDialogs.h"

GuiActions* GuiActions::instance()
{
    static GuiActions instance;
    return &instance;
}

GuiActions::GuiActions(QObject *parent) : QObject(parent)
{
}

void GuiActions::showIssue(int id)
{
    emit instance()->operationRequest(BugManager::Operation_Show, id);
}

void GuiActions::deleteRelation(int id1, int id2)
{
    if (Ori::Dlg::yes(qApp->tr("Delete relation [#%1 - #%2]?").arg(id1).arg(id2)))
    {
        QString res = BugManager::deleteRelation(id1, id2);
        if (!res.isEmpty())
            Ori::Dlg::error(res);
        emit instance()->operationRequest(BugManager::Operation_Update, id1);
        emit instance()->operationRequest(BugManager::Operation_Update, id2);
    }
}
