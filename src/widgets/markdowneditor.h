#ifndef MARKDOWN_EDITOR_H
#define MARKDOWN_EDITOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QPlainTextEdit;
class QTabWidget;
class QTextBrowser;
QT_END_NAMESPACE

class IssueTextView;

class MarkdownEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MarkdownEditor(const QString& editorTabTitle, QWidget *parent = nullptr);

    bool isModified() const;
    QString getText() const;

    QPlainTextEdit* editor() { return _editor; }

    void setFocus();

private slots:
    void tabSwitched(int tabIndex);

private:
    QTabWidget* _tabs;
    QPlainTextEdit* _editor;
    IssueTextView* _preview;
    int _tabIndexEditor, _tabIndexPreview;

    static QLabel* makeHintLabel();
};

#endif // MARKDOWN_EDITOR_H
