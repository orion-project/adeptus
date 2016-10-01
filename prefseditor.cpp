#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QTabWidget>

#include "prefseditor.h"
#include "preferences.h"

bool PrefsEditor::show(QWidget *parent)
{
    PrefsEditor dlg(parent);
    return dlg.exec() == QDialog::Accepted;
}

PrefsEditor::PrefsEditor(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(makeGeneralPage(), tr("General"));

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addWidget(tabs);
    layoutMain->addWidget(buttons);
    setLayout(layoutMain);
}

void PrefsEditor::saveSettings()
{
    Preferences& prefs = Preferences::instance();

    prefs.bugEditorEnableDates = bugEditorEnableDates->isChecked();
    prefs.bugSolverEnableDates = bugSolverEnableDates->isChecked();
    prefs.confirmCancel = confirmCancel->isChecked();
    prefs.openNewBugOnPage = openNewBugOnPage->isChecked();

    prefs.save();
}

QWidget* PrefsEditor::makeGeneralPage()
{
    Preferences& prefs = Preferences::instance();

    confirmCancel = new QCheckBox(tr("Request confirmation when closing editor windows"));
    confirmCancel->setChecked(prefs.confirmCancel);
    openNewBugOnPage = new QCheckBox(tr("Open just created issue in new page"));
    openNewBugOnPage->setChecked(prefs.openNewBugOnPage);

    QVBoxLayout *layoutOptions = new QVBoxLayout;
    layoutOptions->addWidget(confirmCancel);
    layoutOptions->addWidget(openNewBugOnPage);

    QGroupBox *groupOptions = new QGroupBox(tr("Options"));
    groupOptions->setLayout(layoutOptions);


    bugEditorEnableDates = new QCheckBox(tr("Enable date input fields in issue editor dialog"));
    bugSolverEnableDates = new QCheckBox(tr("Enable date input fields in issue solver dialog"));
    bugEditorEnableDates->setChecked(prefs.bugEditorEnableDates);
    bugSolverEnableDates->setChecked(prefs.bugSolverEnableDates);

    QVBoxLayout *layoutService = new QVBoxLayout;
    layoutService->addWidget(bugEditorEnableDates);
    layoutService->addWidget(bugSolverEnableDates);

    QGroupBox *groupService = new QGroupBox(tr("Service options"));
    groupService->setLayout(layoutService);


    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addWidget(groupOptions);
    layoutMain->addWidget(groupService);

    QWidget *page = new QWidget;
    page->setLayout(layoutMain);
    return page;
}
