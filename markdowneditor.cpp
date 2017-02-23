#include <QBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QTextBrowser>
#include <QPlainTextEdit>

#include "markdown.h"
#include "markdowneditor.h"
#include "helpers/OriWidgets.h"

MarkdownEditor::MarkdownEditor(const QString &editorTabTitle, QWidget *parent) : QWidget(parent)
{
    _editor = new QPlainTextEdit;
    Ori::Gui::adjustFont(_editor);

    _preview = new QTextBrowser;

    auto editorTab = new QWidget;
    editorTab->setLayout(Ori::Gui::layoutV(Ori::Gui::layoutSpacing(), Ori::Gui::layoutSpacing(), {_editor, makeHintLabel()}));

    auto previewTab = new QWidget;
    previewTab->setLayout(Ori::Gui::layoutV(Ori::Gui::layoutSpacing(), Ori::Gui::layoutSpacing(), {_preview}));

    _tabs = new QTabWidget;
    connect(_tabs, &QTabWidget::currentChanged, this, &MarkdownEditor::tabSwitched);
    _tabIndexEditor = _tabs->addTab(editorTab, editorTabTitle.isEmpty()? tr("Edit"): editorTabTitle);
    _tabIndexPreview = _tabs->addTab(previewTab, tr("Preview"));

    setLayout(Ori::Gui::layoutH(0, 0, {_tabs}));
}

QLabel* MarkdownEditor::makeHintLabel()
{
    auto label = new QLabel(Markdown::hint());
    auto palette = label->palette();
    auto color = palette.color(QPalette::WindowText);
    color.setAlpha(80);
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);
    return label;
}

bool MarkdownEditor::isModified() const
{
    return _editor->document()->isModified();
}

QString MarkdownEditor::getText() const
{
    return _editor->toPlainText().trimmed();
}

void MarkdownEditor::setFocus()
{
    _tabs->setCurrentIndex(_tabIndexEditor);
    _editor->setFocus();
}

void MarkdownEditor::tabSwitched(int tabIndex)
{
    if (tabIndex == _tabIndexPreview)
        _preview->setHtml(Markdown::process(_editor->toPlainText()));
}
