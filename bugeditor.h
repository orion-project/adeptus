#ifndef BUG_EDITOR_H
#define BUG_EDITOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDataWidgetMapper;
class QDateTimeEdit;
class QLabel;
class QModelIndex;
class QPlainTextEdit;
class QSpinBox;
class QSqlTableModel;
QT_END_NAMESPACE

class MarkdownEditor;

class BugEditor : public QWidget
{
    Q_OBJECT

public:
    explicit BugEditor(QWidget *parentl = 0);
    ~BugEditor();

    static void append(QWidget *parent);
    static void edit(QWidget *parent, int id);

public slots:
    void issueDeleted(int id);

private slots:
    void save();
    void reject();

private:
    QComboBox *comboCategory;
    QComboBox *comboStatus;
    QComboBox *comboSeverity;
    QComboBox *comboPriority;
    QComboBox *comboRepeat;
    QComboBox *comboSolution;
    QDateTimeEdit *dateCreated;
    QDateTimeEdit *dateUpdated;
    QPlainTextEdit *textSummary;
    MarkdownEditor *textExtra;
    QSqlTableModel *tableModel;
    QDataWidgetMapper *mapper;
    QLabel *labelStatus;
    QLabel *labelSolution;
    int mode;
    int currentId;

    void initAppend();
    QString initEdit(int id);
    QString saveNew();
    QString saveEdit();
    QLabel* columnTitle(int columnId);
    void cancelDlg();
};

#endif // BUG_EDITOR_H
