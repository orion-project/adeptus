#include <QtSql>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

#include "mainwindow.h"
#include "bugmanager.h"
#include "bugeditor.h"
#include "bugsolver.h"
#include "bughistory.h"
#include "dicteditor.h"
#include "prefseditor.h"
#include "preferences.h"
#include "issuetable.h"
#include "aboutwindow.h"
#include "operations.h"
#include "startpage.h"
#include "db/db.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"
#include "helpers/OriWindows.h"
#include "helpers/OriLayouts.h"
#include "tools/OriSettings.h"
#include "tools/OriMruList.h"
#include "tools/OriWaitCursor.h"
#include "widgets/OriStylesMenu.h"
#include "widgets/OriMruMenu.h"
#include "widgets/OriBackWidget.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    Ori::Wnd::setWindowIcon(this, ":/window_icons/main");

    mruList = new Ori::MruFileList(this);
    connect(mruList, SIGNAL(clicked(QString)), this, SLOT(openFile(QString)));

    issueTable = new IssueTableWidget(this);
    connect(issueTable, SIGNAL(onFilter()), this, SLOT(updateCounter()));
    connect(issueTable, SIGNAL(onDoubleClick()), this, SLOT(showHistory()));
    connect(issueTable, SIGNAL(onAppendBug()), this, SLOT(appendBug()));

    issueTabs = new QTabWidget;
    issueTabs->setTabsClosable(true);
    connect(issueTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
    setCentralWidget(issueTabs);

    auto back = new Ori::Widgets::BackWidget(":/about/bug", Qt::AlignTop | Qt::AlignRight);
    auto mru = new Ori::Widgets::MruFileListWidget(mruList);
    Ori::Layouts::LayoutH({mru}).useFor(back);
    Ori::Gui::adjustFont(mru);
    issueTabs->addTab(back, tr("Start page"));

    createMenus();
    createStatusBar();

    readSettings();

    setCurrentFile("");
    setUnifiedTitleAndToolBarOnMac(true);

    connect(Operations::instance(), &Operations::issueAdded, this, &MainWindow::issueAdded);
    connect(Operations::instance(), &Operations::issueDeleted, this, &MainWindow::issueDeleted);
    connect(Operations::instance(), &Operations::issueChanged, this, &MainWindow::updateView);
    connect(Operations::instance(), &Operations::relationsChanged, this, &MainWindow::updatePageById);
    connect(Operations::instance(), &Operations::requestShowIssue, this, &MainWindow::openHistoryPage);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    closeCurrentFile();
    writeSettings();
    event->accept();
}

void MainWindow::createMenus()
{
    menuBar()->setNativeMenuBar(false);

    /////////////// File
    QMenu* menuFile = menuBar()->addMenu(tr("&File"));
    menuFile->addAction(tr("New..."), this, SLOT(newFile()));
    menuFile->addAction(tr("Open..."), this, SLOT(openFile()), QKeySequence::Open);
    menuFile->addSeparator();
    auto actionExit = menuFile->addAction(tr("Exit"), this, SLOT(close()), QKeySequence::Quit);
    new Ori::Widgets::MruMenuPart(mruList, menuFile, actionExit, this);

    /////////////// View
    QMenu* menuView = menuBar()->addMenu(tr("View"));
    menuView->addSeparator();
    menuView->addAction(tr("Preferences..."), [this](){ PrefsEditor::show(this); });
    menuView->addSeparator();
    menuView->addMenu(new Ori::Widgets::StylesMenu);

    /////////////// Bug
    menuBug = menuBar()->addMenu(tr("Issue"));
    menuBug->addAction(QIcon(":/tools/append"), tr("New..."), this, SLOT(appendBug()), QKeySequence::New);
    menuBug->addSeparator();
    actionProcessBug = menuBug->addAction(QString(), this, SLOT(processBug()), Qt::Key_F9);
    QAction* actionComment = menuBug->addAction(tr("Comment..."), [this](){ Operations::commentIssue(this->currentId()); }, Qt::Key_F4);
    QAction* actionHistory = menuBug->addAction(tr("History"), this, SLOT(showHistory()), Qt::Key_Return);
    menuBug->addSeparator();
    menuBug->addAction(tr("Edit..."), this, SLOT(editBug()), Qt::Key_F2);
    menuBug->addAction(tr("Delete"), [this](){ Operations::deleteIssue(this->currentId()); });
    menuBug->addSeparator();
    menuBug->addAction(tr("Make Relation..."), [this](){ Operations::makeRelation(this->currentId()); });

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionProcessBug);
    contextMenu->addAction(actionComment);
    contextMenu->addAction(actionHistory);
    
    connect(menuBug, SIGNAL(aboutToShow()), this, SLOT(menuBugOpened()));
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(menuBugOpened()));

    /////////////// Dicts
    menuDicts = menuBar()->addMenu(tr("Dictionaries"));
    foreach (int dictId, BugManager::dictionaryIds())
        menuDicts->addAction(BugManager::columnTitle(dictId), this, SLOT(editDictionary()))->setData(dictId);

    /////////////// Help
    QMenu* menuHelp = menuBar()->addMenu(tr("Help"));
    menuHelp->addAction(tr("About ") + qApp->applicationName(), this, SLOT(about()));
}

void MainWindow::createStatusBar()
{
    statusBar()->addWidget(statusTotalCount = new QLabel);
    statusBar()->addWidget(statusOpenedCount = new QLabel);
    statusBar()->addWidget(statusDisplayCount = new QLabel);
    statusBar()->addWidget(statusFileName = new QLabel);
    statusBar()->showMessage(tr("Ready"));

    statusTotalCount->setMargin(2);
    statusOpenedCount->setMargin(2);
    statusDisplayCount->setMargin(2);
    statusFileName->setMargin(2);
}

void MainWindow::writeSettings()
{
    Ori::Settings s;
    s.storeWindowGeometry("mainWindow", this);

    s.resetGroup();
    s.setValue("style", qApp->style()->objectName());
}

void MainWindow::readSettings()
{
    Ori::Settings s;
    s.restoreWindowGeometry("mainWindow", this, QSize(800, 600));

    Preferences::instance().load(s);

    s.resetGroup();
    qApp->setStyle(s.strValue("style"));
    mruList->load(s.settings());
}

void MainWindow::closeCurrentFile()
{
    if (issueTable->isFilterChanged()) issueTable->saveFilters();
    issueTable->close();
}

void MainWindow::newFile()
{
    QString fileName = QFileDialog::getSaveFileName(
                this, tr("Create Database"), QString(), tr(BUGS_FILES));
    if (fileName.isEmpty())
        return;

    Ori::WaitCursor c;

    closeCurrentFile();

    QString res = BugManager::newDatabase(fileName);
    if (res.isEmpty())
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("Database created"), 2000);
    }
    else
    {
        QMessageBox::critical(this, qApp->applicationName(),
            tr("Unable to create a database.\n\n%1").arg(res));
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Database"), QString(), tr(BUGS_FILES));
    if (!fileName.isEmpty())
        openFile(fileName);
}

void MainWindow::openFile(const QString &fileName)
{
    Ori::WaitCursor c;

    closeCurrentFile();

    QString res = BugManager::openDatabase(fileName);
    if (res.isEmpty())
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("Database loaded"), 2000);
    }
    else
    {
        QMessageBox::critical(this, qApp->applicationName(),
            tr("Unable to load database.\n\n%1").arg(res));
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    if (!currentFile.isEmpty())
    {
        setWindowTitle(QFileInfo(currentFile).fileName() % " - " % qApp->applicationName());
        closeTabs();
        mruList->append(fileName);
        statusFileName->setText(QDir::toNativeSeparators(currentFile));
        tableModel = issueTable->update();
        issueTable->loadFilters();
        issueTable->contextMenu = contextMenu;
        issueTabs->addTab(issueTable, tr("Issues"));
        updateCounter();
    }
    else
        setWindowTitle(qApp->applicationName());

    updateActions();
}

void MainWindow::editDictionary()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    int dictId = action->data().toInt();
    if (DictEditor::show(this, action->text(), dictId))
    {
        BugManager::updateDictionaryCash(dictId);
        issueTable->adjustHeader();
        updatePageById(-1);
    }
}

void MainWindow::menuBugOpened()
{
    actionProcessBug->setText(BugManager::operationTitle(currentStatus()));
}

void MainWindow::appendBug()
{
    if (!tableModel) return;
    BugEditor::append(this);
}

void MainWindow::issueAdded(int id)
{
    if (BugManager::isInvalid(id)) return;

    tableModel->select();

    // It's not fine to fetch the whole table, but I
    // don't know how to focus new row after addition.
    while (tableModel->canFetchMore())
        tableModel->fetchMore();

    updateCounter();
    issueTable->adjustHeader();
    issueTable->setSelectedId(id);

    if (Preferences::instance().openNewBugOnPage)
        openHistoryPage(id);
    else
        issueTabs->setCurrentIndex(0);
}

void MainWindow::issueDeleted(int id)
{
    tableModel->select();
    issueTable->adjustHeader();
    updateCounter();
    updatePagesByRelatedId(id);
    closeTab(indexOfId(id));
}

void MainWindow::editBug()
{
    BugEditor::edit(this, currentId());
}

void MainWindow::processBug()
{
    BugSolver::process(this, currentId(), currentStatus());
}

void MainWindow::showHistory()
{
    if (!tableModel || page()) return;

    openHistoryPage(issueTable->selectedId());
}

void MainWindow::openHistoryPage(int id)
{
    if (id < 0) return;
    BugHistory* history = pageById(id);
    if (!history)
    {
        history = new BugHistory(id);
        issueTabs->addTab(history, "");
        updatePageById(id);
    }
    issueTabs->setCurrentWidget(history);
}

void MainWindow::updateCounter()
{
    if (tableModel)
    {
        int total, opened, displayed;
        QString res = BugManager::countBugs(total, opened, displayed, tableModel->filter());
        if (!res.isEmpty())
        {
            statusTotalCount->setText(res);
            statusOpenedCount->setText("");
            statusDisplayCount->setText("");
        }
        else
        {
            statusTotalCount->setText(tr("Total Issues: %1").arg(total));
            statusOpenedCount->setText(tr("Opened: %1").arg(opened));
            statusDisplayCount->setText(tr("Displayed: %1").arg(displayed));
        }
    }
    else
    {
        statusTotalCount->setText("");
        statusOpenedCount->setText("");
        statusDisplayCount->setText("");
    }
}

void MainWindow::tabCloseRequested(int index)
{
    if (index > 0)
        closeTab(index);

    auto hist = page();
    if (hist) hist->setFocus();
    else issueTable->setFocus();
}

void MainWindow::closeTabs()
{
    while (issueTabs->count() > 0)
        closeTab(issueTabs->count()-1);
}

void MainWindow::closeTab(int index)
{
    if (index >= 0 && index < issueTabs->count())
    {
        QWidget *page = issueTabs->widget(index);
        issueTabs->removeTab(index);
        if (page != issueTable) delete page;
    }
}

void MainWindow::about()
{
    (new AboutWindow(this))->show();
}

BugHistory* MainWindow::page(int index)
{
    return dynamic_cast<BugHistory*>(index < 0? issueTabs->currentWidget(): issueTabs->widget(index));
}

BugHistory* MainWindow::pageById(int id)
{
    for (int i = 1; i < issueTabs->count(); i++)
    {
        BugHistory *h = page(i);
        if (h && h->id() == id) return h;
    }
    return NULL;
}

int MainWindow::indexOfId(int id)
{
    for (int i = 1; i < issueTabs->count(); i++)
        if (page(i)->id() == id) return i;
    return -1;
}

void MainWindow::updatePageById(int id)
{
    for (int i = 1; i < issueTabs->count(); i++)
    {
        BugHistory *h = page(i);
        if (id < 0 || h->id() == id)
        {
            h->populate();
            issueTabs->setTabText(i, QString("[#%1] %2...").arg(h->id()).arg(h->title().left(16)));
            issueTabs->setTabToolTip(i, h->title());
            if (id > 0) return;
        }
    }
}

void MainWindow::updatePagesByRelatedId(int id)
{
    for (int i = 1; i < issueTabs->count(); i++)
    {
        BugHistory *h = page(i);
        if (h->relatedIds().contains(id))
            h->populate();
    }
}

int MainWindow::currentId()
{
    if (!tableModel) return -1;
    BugHistory *h = page();
    return h? h->id(): issueTable->selectedId();
}

int MainWindow::currentStatus()
{
    if (!tableModel) return -1;
    BugHistory *h = page();
    if (h) return h->status();
    int row = issueTable->selectedRow();
    if (row < 0) return -1;
    return tableModel->record(row).field(COL_STATUS).value().toInt();
}

void MainWindow::updateView(int id)
{
    tableModel->select();
    issueTable->setSelectedId(id);
    updatePageById(id);
    updatePagesByRelatedId(id);
    updateCounter();
}

void MainWindow::updateActions()
{
    menuBug->setEnabled(tableModel);
    menuDicts->setEnabled(tableModel);
}
