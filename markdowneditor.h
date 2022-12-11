#ifndef MARKDOWNEDITOR_H
#define MARKDOWNEDITOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QTabWidget;
QT_END_NAMESPACE

class IssueTextEdit;
class IssueTextView;

class MarkdownEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MarkdownEditor(const QString& editorTabTitle, QWidget *parent = 0);

    bool isModified() const;
    QString getText() const;

    QPlainTextEdit* editor();

    void setFocus();

private slots:
    void tabSwitched(int tabIndex);

private:
    QTabWidget* _tabs;
    IssueTextEdit* _editor;
    IssueTextView* _preview;
    int _tabIndexEditor, _tabIndexPreview;
};

#endif // MARKDOWNEDITOR_H
