#include "preferences.h"
#include "tools/OriSettings.h"

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
