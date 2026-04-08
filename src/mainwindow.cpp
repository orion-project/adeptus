#include "mainwindow.h"

#include "bugmanager.h"
#include "bugeditor.h"
#include "bugsolver.h"
#include "dicteditor.h"
#include "Preferences.h"
#include "aboutwindow.h"
#include "operations.h"
#include "db/Db.h"
#include "db/Dicts.h"
#include "pages/HistoryPage.h"
#include "pages/TablePage.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"
#include "helpers/OriWindows.h"
#include "helpers/OriLayouts.h"
#include "tools/OriSettings.h"
#include "tools/OriMruList.h"
#include "widgets/OriStylesMenu.h"
#include "widgets/OriMruMenu.h"
#include "widgets/OriBackWidget.h"

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    Ori::Wnd::setWindowIcon(this, ":/window_icons/main");

    mruList = new Ori::MruFileList(this);
    connect(mruList, SIGNAL(clicked(QString)), this, SLOT(openFile(QString)));

    tablePage = new TablePage(this);
    tablePage->setVisible(false);
    connect(tablePage, SIGNAL(onFilter()), this, SLOT(updateCounter()));
    connect(tablePage, SIGNAL(onDoubleClick()), this, SLOT(showHistory()));
    connect(tablePage, SIGNAL(onAppendBug()), this, SLOT(appendBug()));

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
    Db::close();
}

void MainWindow::createMenus()
{
    menuBar()->setNativeMenuBar(false);

    // File
    QMenu* menuFile = menuBar()->addMenu(tr("&File"));
    menuFile->addAction(tr("New..."), this, SLOT(newFile()));
    menuFile->addAction(tr("Open..."), QKeySequence::Open, this, SLOT(openFile()));
    menuFile->addSeparator();
    auto actionExit = menuFile->addAction(tr("Exit"), QKeySequence::Quit, this, SLOT(close()));
    new Ori::Widgets::MruMenuPart(mruList, menuFile, actionExit, this);

    // View
    QMenu* menuView = menuBar()->addMenu(tr("View"));
    menuView->addSeparator();
    menuView->addAction(tr("Preferences..."), this, [this](){ Preferences::instance().edit(); });
    menuView->addSeparator();
    menuView->addMenu(new Ori::Widgets::StylesMenu);

    // Bug
    menuBug = menuBar()->addMenu(tr("Issue"));
    menuBug->addAction(QIcon(":/tools/append"), tr("New..."), QKeySequence::New, this, SLOT(appendBug()));
    menuBug->addSeparator();
    actionProcessBug = menuBug->addAction(QString(), Qt::Key_F9, this, SLOT(processBug()));
    QAction* actionComment = menuBug->addAction(tr("Comment..."), Qt::Key_F4, this, [this](){ Operations::commentIssue(this->currentId()); });
    QAction* actionHistory = menuBug->addAction(tr("History"), Qt::Key_Return, this, SLOT(showHistory()));
    menuBug->addSeparator();
    menuBug->addAction(tr("Edit..."), Qt::Key_F2, this, SLOT(editBug()));
    menuBug->addAction(tr("Delete"), this, [this](){ Operations::deleteIssue(this->currentId()); });
    menuBug->addSeparator();
    menuBug->addAction(tr("Make Relation..."), this, [this](){ Operations::makeRelation(this->currentId()); });

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionProcessBug);
    contextMenu->addAction(actionComment);
    contextMenu->addAction(actionHistory);
    
    connect(menuBug, SIGNAL(aboutToShow()), this, SLOT(menuBugOpened()));
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(menuBugOpened()));

    // Dicts
    menuDicts = menuBar()->addMenu(tr("Dictionaries"));
    foreach (int dictId, Db::Dicts::dictIds())
        menuDicts->addAction(BugManager::columnTitle(dictId), this, SLOT(editDictionary()))->setData(dictId);

    // Help
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
    if (tablePage->isFilterChanged()) tablePage->saveFilters();
    tablePage->close();
}

void MainWindow::newFile()
{
    QString fileName = QFileDialog::getSaveFileName(
                this, tr("Create Database"), QString(), Db::fileFilter());
    if (fileName.isEmpty())
        return;

    closeCurrentFile();

    QString res = Db::create(fileName);
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
                this, tr("Open Database"), QString(), Db::fileFilter());
    if (!fileName.isEmpty())
        openFile(fileName);
}

void MainWindow::openFile(const QString &fileName)
{
    closeCurrentFile();

    QString res = Db::open(fileName);
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
        tableModel = tablePage->update();
        tablePage->setVisible(true);
        tablePage->loadFilters();
        tablePage->contextMenu = contextMenu;
        issueTabs->addTab(tablePage, tr("Issues"));
        updateCounter();
    }
    else
    {
        tablePage->setVisible(false);
        setWindowTitle(qApp->applicationName());
    }

    updateActions();
}

void MainWindow::editDictionary()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    int dictId = action->data().toInt();
    if (DictEditor::show(this, action->text(), dictId))
    {
        Db::Dicts::updateCache(dictId);
        tablePage->adjustHeader();
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
    tablePage->adjustHeader();
    tablePage->setSelectedId(id);

    if (Preferences::instance().openNewBugOnPage)
        openHistoryPage(id);
    else
        issueTabs->setCurrentIndex(0);
}

void MainWindow::issueDeleted(int id)
{
    tableModel->select();
    tablePage->adjustHeader();
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

    openHistoryPage(tablePage->selectedId());
}

void MainWindow::openHistoryPage(int id)
{
    if (id < 0) return;
    HistoryPage* history = pageById(id);
    if (!history)
    {
        history = new HistoryPage(id);
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
    else tablePage->setFocus();
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
        if (page != tablePage) delete page;
    }
}

void MainWindow::about()
{
    (new AboutWindow(this))->show();
}

HistoryPage* MainWindow::page(int index)
{
    return dynamic_cast<HistoryPage*>(index < 0? issueTabs->currentWidget(): issueTabs->widget(index));
}

HistoryPage* MainWindow::pageById(int id)
{
    for (int i = 1; i < issueTabs->count(); i++)
    {
        HistoryPage *h = page(i);
        if (h && h->id() == id) return h;
    }
    return nullptr;
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
        HistoryPage *h = page(i);
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
        HistoryPage *h = page(i);
        if (h->relatedIds().contains(id))
            h->populate();
    }
}

int MainWindow::currentId()
{
    if (!tableModel) return -1;
    HistoryPage *h = page();
    return h? h->id(): tablePage->selectedId();
}

int MainWindow::currentStatus()
{
    if (!tableModel) return -1;
    HistoryPage *h = page();
    if (h) return h->status();
    int row = tablePage->selectedRow();
    if (row < 0) return -1;
    return tableModel->record(row).field(COL_STATUS).value().toInt();
}

void MainWindow::updateView(int id)
{
    tableModel->select();
    tablePage->setSelectedId(id);
    updatePageById(id);
    updatePagesByRelatedId(id);
    updateCounter();
}

void MainWindow::updateActions()
{
    menuBug->setEnabled(tableModel);
    menuDicts->setEnabled(tableModel);
}
