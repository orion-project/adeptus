#include "filterpanel.h"
#include "bugmanager.h"
#include "widgets/OriFlowLayout.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDebug>
#include <QInputDialog>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSqlTableModel>

FilterView::FilterView(const QString& name, int dictId) : QWidget(), _name(name), _dictId(dictId)
{
    valueCombo = new QComboBox;
    valueCombo->setMaxVisibleItems(24);
    valueCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    valueCombo->setEnabled(false);

    conditionCombo = new QComboBox;
    conditionCombo->addItems({"<", "<=", "=", "!=", ">=", ">"});
    conditionCombo->setCurrentIndex(2);
    conditionCombo->setEnabled(false);

    enabledFlag = new QCheckBox(name);
    connect(enabledFlag, SIGNAL(toggled(bool)), valueCombo, SLOT(setEnabled(bool)));
    connect(enabledFlag, SIGNAL(toggled(bool)), conditionCombo, SLOT(setEnabled(bool)));

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
#ifndef Q_OS_MAC
    layout->setSpacing(3);
#endif
    layout->addWidget(enabledFlag);
    layout->addWidget(conditionCombo);
    layout->addWidget(valueCombo);
}

void FilterView::updateModel()
{
    valueCombo->setModel(BugManager::dictionary(_dictId));
    valueCombo->setModelColumn(DICT_COL_TITLE);
}

void FilterView::collectTo(IssueFilters *filters)
{
    filters->add(_name,
                 enabledFlag->isChecked(),
                 conditionCombo->currentText(),
                 WidgetHelper::selectedId(valueCombo).toInt());
}

void FilterView::populateFrom(IssueFilters *filters)
{
    for (const IssueFilter& filter : filters->filters)
        if (filter.name == _name)
        {
            enabledFlag->setChecked(filter.check);
            WidgetHelper::selectText(conditionCombo, filter.condition);
            WidgetHelper::selectId(valueCombo, filter.value);
        }
}

//-----------------------------------------------------------------------------------------------

FilterPresets::FilterPresets(FilterPanel *parent) : QObject(parent)
{
    _panel = parent;
    _menu = new QMenu(parent);
    _actionSave = Ori::Gui::action(tr("Save Filter..."), this, SLOT(save()));
    _actionDelete = Ori::Gui::action(tr("Delete Filter..."), this, SLOT(remove()));
}

void FilterPresets::show(const QPoint& pos) { _menu->exec(pos); }

void FilterPresets::load()
{
    for (const FilterPreset& it: _items) delete it.action;
    _items.clear();
    _menu->clear();

    auto presets = IssueFiltersPreset::loadPresets();
    if (!presets.ok())
        return Ori::Dlg::error(tr("Unable to load saved filters\n\n%1").arg(presets.error()));

    QMapIterator<int, QString> it(presets.result());
    while (it.hasNext())
    {
        it.next();
        FilterPreset preset;
        preset.id = it.key();
        preset.title = it.value();
        makeAction(preset);
        _menu->addAction(preset.action);
        _items.append(preset);
    }
    _separator = _menu->addSeparator();
    _menu->addAction(_actionSave);
    _menu->addAction(_actionDelete);
    updateDeleteAction();
}

void FilterPresets::save()
{
    QString title = QInputDialog::getText(qApp->activeWindow(), tr("Save Filter"), tr("Enter new filter title:"));
    if (title.isEmpty()) return;

    for (const FilterPreset& it: _items)
        if (it.title == title)
            return Ori::Dlg::info(tr("There is a filter having this title already"));

    auto id = IssueFiltersPreset::generatePresetId();
    if (!id.ok())
        return Ori::Dlg::error(tr("Unable to generate id for new filter\n\n%1").arg(id.error()));

    IssueFiltersPreset filters(id.result(), title);
    _panel->collectFilters(&filters);
    QString res = filters.save();
    if (!res.isEmpty())
        return Ori::Dlg::error(tr("Unable to save filter\n\n%1").arg(res));

    FilterPreset preset;
    preset.id = id.result();
    preset.title = title;
    makeAction(preset);
    _menu->insertAction(_separator, preset.action);
    _items.append(preset);
    updateDeleteAction();
}

void FilterPresets::makeAction(FilterPreset& preset)
{
    preset.action = Ori::Gui::action(preset.title, this, SLOT(apply()));
    preset.action->setData(preset.id);
}

void FilterPresets::remove()
{
    QListWidget l;
    for (FilterPreset& it: _items)
        l.addItem(it.title);
    l.setCurrentRow(0);

    if (Ori::Dlg::showDialogWithPrompt(Qt::Vertical, tr("Select filter to delete:"), &l, nullptr))
    {
        int i = l.currentRow();

        QString res = IssueFiltersPreset::deletePreset(_items.at(i).id);
        if (!res.isEmpty())
            return Ori::Dlg::error(tr("Error while delete filter\n\n%1").arg(res));

        delete _items.at(i).action;
        _items.removeAt(i);
        updateDeleteAction();
    }
}

void FilterPresets::apply()
{
    auto action = qobject_cast<QAction*>(sender());
    if (!action) return;

    IssueFiltersPreset preset(action->data().toInt());
    QString res = preset.load();
    if (!res.isEmpty())
        return Ori::Dlg::error(tr("Unable to load filter\n\n%1").arg(res));

    _panel->displayFilters(&preset);
    emit _panel->filterChanged();
}

void FilterPresets::updateDeleteAction() { _actionDelete->setEnabled(!_items.isEmpty()); }

//-----------------------------------------------------------------------------------------------

FilterPanel::FilterPanel() : QWidget()
{
    _filters = { new FilterView("Status", COL_STATUS),
                 new FilterView("Solution", COL_SOLUTION),
                 new FilterView("Category", COL_CATEGORY),
                 new FilterView("Severity", COL_SEVERITY),
                 new FilterView("Priority", COL_PRIORITY) };

    auto applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyButtonClicked()));

    auto filterLayout = new Ori::Widgets::FlowLayout(6, 24, 6);
    for (auto filter : _filters)
        filterLayout->addWidget(filter);
    filterLayout->addWidget(applyButton);

    auto mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->addLayout(filterLayout);
    mainLayout->addStretch();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    setLayout(mainLayout);

    _presets = new FilterPresets(this);
}

void FilterPanel::load()
{
    IssueFilters filters;
    QString res = filters.load();
    if (!res.isEmpty())
        return Ori::Dlg::error(tr("Unable to load filters\n\n%1").arg(res));

    displayFilters(&filters);
    emit filterChanged();

    _presets->load();
}

void FilterPanel::applyButtonClicked()
{
    emit filterChanged();
}

void FilterPanel::updateModels()
{
    for (auto filter : _filters)
        filter->updateModel();
}

void FilterPanel::collectFilters(IssueFilters *filters)
{
    for (auto filter : _filters)
        filter->collectTo(filters);
}

void FilterPanel::displayFilters(IssueFilters *filters)
{
    for (auto filter : _filters)
        filter->populateFrom(filters);
}

void FilterPanel::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
    _presets->show(event->globalPos());
}

