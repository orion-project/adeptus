#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QPushButton;
class QSqlTableModel;
class QTabWidget;
class QVBoxLayout;
QT_END_NAMESPACE

class IssueTableWidget;
class BugHistory;
namespace Ori {
class MruFileList;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void openFile();
    void openFile(const QString &fileName);
    void editDictionary();
    void appendBug();
    void deleteBug();
    void editBug();
    void processBug();
    void menuBugOpened();
    void showHistory();
    void updateCounter();
    void tabCloseRequested(int index);
    void about();
    void processOperation(int operation, int id);
    void updateView(int id);
    void bugAdded(int id);

private:
    QMenu *menuBug, *menuDicts, *menuDebug, *contextMenu;
    QAction *actionProcessBug;
    QLabel *statusTotalCount, *statusOpenedCount, *statusDisplayCount, *statusFileName;

    QString currentFile;
    QTabWidget *issueTabs;
    QSqlTableModel *tableModel = nullptr;
    IssueTableWidget *issueTable;
    Ori::MruFileList *mruList;

    void createMenus();
    void createStatusBar();
    void writeSettings();
    void readSettings();
    void updateFilterPanel();
    void updateActions();
    void setCurrentFile(const QString &fileName);
    void closeCurrentFile();
    void saveFilters();
    void closeTabs();
    void closeTab(int index);
    int currentId();
    int currentStatus();
    void showHistory(int id);
    BugHistory* page(int index = -1);
    BugHistory* pageById(int id);
    int indexOfId(int id);
    void updatePageById(int id);
    void updatePagesByRelatedId(int id);
};

#endif // MAINWINDOW_H
