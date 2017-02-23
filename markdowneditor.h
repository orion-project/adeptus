#ifndef MARKDOWNEDITOR_H
#define MARKDOWNEDITOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QPlainTextEdit;
class QTabWidget;
class QTextBrowser;
QT_END_NAMESPACE

class MarkdownEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MarkdownEditor(const QString& editorTabTitle, QWidget *parent = 0);

    bool isModified() const;
    QString getText() const;

    QPlainTextEdit* editor() { return _editor; }

    void setFocus();

private slots:
    void tabSwitched(int tabIndex);

private:
    QTabWidget* _tabs;
    QPlainTextEdit* _editor;
    QTextBrowser* _preview;
    int _tabIndexEditor, _tabIndexPreview;

    static QLabel* makeHintLabel();
};

#endif // MARKDOWNEDITOR_H
