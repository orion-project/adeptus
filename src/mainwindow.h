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

class TablePage;
class HistoryPage;
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
    void editBug();
    void processBug();
    void menuBugOpened();
    void showHistory();
    void updateCounter();
    void tabCloseRequested(int index);
    void about();
    void updateView(int id);

private:
    QMenu *menuBug, *menuDicts, *menuDebug, *contextMenu;
    QAction *actionProcessBug;
    QLabel *statusTotalCount, *statusOpenedCount, *statusDisplayCount, *statusFileName;

    QString currentFile;
    QTabWidget *issueTabs;
    QSqlTableModel *tableModel = nullptr;
    TablePage *tablePage;
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
    void openHistoryPage(int id);
    HistoryPage* page(int index = -1);
    HistoryPage* pageById(int id);
    int indexOfId(int id);
    void issueAdded(int id);
    void issueDeleted(int id);
    void updatePageById(int id);
    void updatePagesByRelatedId(int id);
};

#endif // MAINWINDOW_H
