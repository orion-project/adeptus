#include "aboutwindow.h"
#include "widgets/OriLabels.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>

AboutWindow::AboutWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(qApp->applicationName());

    QPixmap bckgnd(":/about/main");
    setMaximumSize(bckgnd.size());
    setMinimumSize(bckgnd.size());
    resize(bckgnd.size());

    auto p = palette();
    p.setBrush(QPalette::Background, QBrush(bckgnd));
    setPalette(p);

    auto labelVersion = new QLabel(QString("%1.%2.%3")
        .arg(APP_VER_MAJOR).arg(APP_VER_MINOR).arg(APP_VER_BUILD));
    labelVersion->setStyleSheet("font-family: Times New Roman; font-size: 40px; "
                                "font-weight: bold; color: #523450");

    auto layoutVersion = new QHBoxLayout;
    layoutVersion->addStretch();
    layoutVersion->addWidget(labelVersion);
    layoutVersion->addSpacing(6);

    QString build(SVN_REV);
    int pos = build.indexOf(':');
    if (pos > 0) build = build.right(build.length()-pos-1);

    auto labelBuild = new QLabel(QString("Build: %1 (%2 %3)").arg(build)
                                    .arg(QString(BUILDDATE).trimmed())
                                    .arg(QString(BUILDTIME).trimmed()));
    labelBuild->setStyleSheet("font-family: Times New Roman; font-size: 20px; color: white");

    auto layoutBuild = new QHBoxLayout;
    layoutBuild->addStretch();
    layoutBuild->addWidget(labelBuild);
    layoutBuild->addSpacing(6);


    QLabel *labelCopyright = new QLabel("Chunosov N.I. (c) 2012-2016");
    labelCopyright->setStyleSheet("font-family: Times New Roman; font-size: 14px; color: #523450");

    auto labelWww = new Ori::Widgets::ClickableLabel(APP_WWW);
    labelWww->setStyleSheet("font-family: Times New Roman; font-size: 14px; color: #523450");
    connect(labelWww, SIGNAL(clicked()), this, SLOT(linkWwwClicked()));

    auto labelMail = new Ori::Widgets::ClickableLabel(APP_MAIL);
    labelMail->setStyleSheet("font-family: Times New Roman; font-size: 14px; color: #523450");
    connect(labelMail, SIGNAL(clicked()), this, SLOT(linkMailClicked()));

    QVBoxLayout *layoutContacts = new QVBoxLayout;
    layoutContacts->setMargin(0);
    layoutContacts->setSpacing(0);
    layoutContacts->addStretch();
    layoutContacts->addWidget(labelCopyright);
    layoutContacts->addWidget(labelWww);
    layoutContacts->addWidget(labelMail);

    auto labelQt = new Ori::Widgets::ClickableLabel(QString("Based on Qt %1").arg(QT_VERSION_STR));
    labelQt->setStyleSheet("font-family: Times New Roman; font-size: 20px; color: #8e7e8c; margin-right: 1px");
    connect(labelQt, SIGNAL(clicked()), qApp, SLOT(aboutQt()));

    QVBoxLayout *layoutQt = new QVBoxLayout;
    layoutQt->setMargin(0);
    layoutQt->addStretch();
    layoutQt->addWidget(labelQt);

    QHBoxLayout *layoutCopyright = new QHBoxLayout;
    layoutCopyright->setMargin(0);
    layoutCopyright->addLayout(layoutContacts);
    layoutCopyright->addStretch();
    layoutCopyright->addLayout(layoutQt);

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setSpacing(0);
    layoutMain->setMargin(6);
    layoutMain->addSpacing(80);
    layoutMain->addLayout(layoutBuild);
    layoutMain->addLayout(layoutVersion);
    layoutMain->addStretch();
    layoutMain->addLayout(layoutCopyright);
    setLayout(layoutMain);
}

void AboutWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) close();
}

void AboutWindow::linkWwwClicked()
{
    QDesktopServices::openUrl(QUrl(APP_WWW));
}


void AboutWindow::linkMailClicked()
{
    QDesktopServices::openUrl(QUrl(QString("mailto:") + APP_MAIL));
}

