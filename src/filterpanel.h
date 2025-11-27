#ifndef FILTERPANEL_H
#define FILTERPANEL_H

#include <QWidget>
#include <QAction>

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QContextMenuEvent;
class QMenu;
QT_END_NAMESPACE

class IssueFilters;

//-----------------------------------------------------------------------------------------------

class FilterView : public QWidget
{
    Q_OBJECT

public:
    FilterView(const QString& name, int dictId);

    void updateModel();
    void collectTo(IssueFilters *filters);
    void populateFrom(IssueFilters *filters);

private:
    QString _name;
    int _dictId;
    QCheckBox *enabledFlag;
    QComboBox *conditionCombo;
    QComboBox *valueCombo;
};

//-----------------------------------------------------------------------------------------------

struct FilterPreset
{
    int id;
    QString title;
    QAction* action;
};

//-----------------------------------------------------------------------------------------------

class FilterPanel;

class FilterPresets : public QObject
{
    Q_OBJECT

public:
    FilterPresets(FilterPanel *parent);

    void load();
    void show(const QPoint&);
    QMenu* menu() const { return _menu; }

private:
    FilterPanel* _panel;
    QList<FilterPreset> _items;
    QAction *_actionSave, *_actionDelete, *_separator;
    QMenu* _menu;

    void updateDeleteAction();
    void makeAction(FilterPreset& preset);

private slots:
    void save();
    void remove();
    void apply();
};

//-----------------------------------------------------------------------------------------------

class FilterPanel : public QWidget
{
    Q_OBJECT

public:
    FilterPanel();

    void load();
    void updateModels();
    void collectFilters(IssueFilters *filters);
    void displayFilters(IssueFilters *filters);

signals:
    void filterChanged();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QList<FilterView*> _filters;
    FilterPresets* _presets;

private slots:
    void applyButtonClicked();
};

#endif // FILTERPANEL_H
