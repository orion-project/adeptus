#ifndef SETTINGSEDITOR_H
#define SETTINGSEDITOR_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

class PrefsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PrefsEditor(QWidget *parent = 0);
    static bool show(QWidget *parent);
    
private slots:
    void saveSettings();
    
private:
    QCheckBox *bugEditorEnableDates, *bugSolverEnableDates;
    QCheckBox *confirmCancel, *openNewBugOnPage;

    QWidget* makeGeneralPage();
};

#endif // SETTINGSEDITOR_H
