#include "Preferences.h"

#include "dialogs/OriConfigDlg.h"
#include "tools/OriSettings.h"

using namespace Ori::Dlg;

Preferences Preferences::preferences;

Preferences::Preferences()
{
}

Preferences& Preferences::instance()
{
    return preferences;
}

Preferences* Preferences::instancePtr()
{
    return &preferences;
}

void Preferences::load(Ori::Settings &s)
{
    s.resetGroup();
    bugEditorEnableDates = s.value("bugEditorEnableDates", false).toBool();
    bugSolverEnableDates = s.value("bugSolverEnableDates", false).toBool();
    confirmCancel = s.value("confirmCancel", true).toBool();
    openNewBugOnPage = s.value("openNewBugOnPage", true).toBool();
}

void Preferences::save(Ori::Settings &s)
{
    s.resetGroup();
    s.setValue("bugEditorEnableDates", bugEditorEnableDates);
    s.setValue("bugSolverEnableDates", bugSolverEnableDates);
    s.setValue("confirmCancel", confirmCancel);
    s.setValue("openNewBugOnPage", openNewBugOnPage);
}

void Preferences::save()
{
    Ori::Settings s;
    save(s);
}

#define PAGE 1

bool Preferences::edit()
{
    ConfigDlgOpts opts;
    opts.objectName = "AppSettingsDlg";
    opts.pageIconSize = 32;
    opts.pages = {
        ConfigPage(PAGE, QObject::tr("General"), ":/tools/settings")
    };
    opts.items = {
        new ConfigItemSection(PAGE, QObject::tr("Options")),
        new ConfigItemBool(PAGE, QObject::tr("Request confirmation when closing editor windows"), &confirmCancel),
        new ConfigItemBool(PAGE, QObject::tr("Open just created issue in new page"), &openNewBugOnPage),
        
        new ConfigItemSpace(PAGE, 12),
        new ConfigItemSection(PAGE, QObject::tr("Service options")),
        new ConfigItemBool(PAGE, QObject::tr("Enable date input fields in issue editor dialog"), &bugEditorEnableDates),
        new ConfigItemBool(PAGE, QObject::tr("Enable date input fields in issue solver dialog"), &bugSolverEnableDates),
    };
    if (ConfigDlg::edit(opts))
    {
        save();
        return true;
    }
    return false;
}