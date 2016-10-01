#ifndef BUG_SOLVER_H
#define BUG_SOLVER_H

#include <QWidget>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDataWidgetMapper;
class QDateTime;
class QDateTimeEdit;
class QLabel;
class QModelIndex;
class QPlainTextEdit;
class QSpinBox;
class QSqlTableModel;
QT_END_NAMESPACE

class BugSolver : public QWidget
{
    Q_OBJECT

public:
    explicit BugSolver(QWidget *parent);
    ~BugSolver();

    static void comment(QWidget *parent, int id);
    static void process(QWidget *parent, int id, int currentStatus);

    void setIcon(const QString& path);

public slots:
    void bugDeleted(int id);

private slots:
    void save();
    void reject();

private:
    QSqlTableModel *tableModel;
    QDataWidgetMapper *mapper;
    QPlainTextEdit *textComment;
    QComboBox *comboStatus;
    QComboBox *comboSolution;
    QDateTimeEdit *dateEvent;
    QLabel* _labelIcon;

    int currentId;
    QVariant oldStatus;
    QVariant oldSolution;

    static BugSolver* initWindow(QWidget *parent, int id, const QString& title);
    static void solve(QWidget *parent, int id);
    static void close(QWidget *parent, int id);
    static void reopen(QWidget *parent, int id);

    QString initWindow(int id);
    QString addHistroyItem(int eventNum, int changedParam, QVariant &oldValue, QVariant &newValue, bool doComment);
    QString saveIssue();
};

#endif // BUG_SOLVER_H
