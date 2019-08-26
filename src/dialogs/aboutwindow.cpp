#include "aboutwindow.h"

#include "orion/widgets/OriLabels.h"
#include "orion/helpers/OriLayouts.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>

#define FONT_NAME "Times New Roman"
#define FONT_COLOR "#523450"
#define STYLE_INFO "font-family:" FONT_NAME "; font-size: 14px; color:" FONT_COLOR
#define STYLE_VERSION "font-family:" FONT_NAME "; font-size: 40px; font-weight: bold; color:" FONT_COLOR
#define STYLE_BUILD "font-family: " FONT_NAME "; font-size: 20px; color: white"
#define STYLE_QT "font-family: " FONT_NAME "; font-size: 20px; color: #8e7e8c; margin-right: 1px"

#define APP_COPYRIGHT "Chunosov N.I. (c) 2012-2019"
#define APP_WWW "http://github.com/orion-project/adeptus"
#define APP_MAIL "adeptus@orion-project.org"

using namespace Ori::Layouts;

AboutWindow::AboutWindow(QWidget *parent) : QWidget(parent)
{
    setupAutoDeletableDialog();
    setBackImageAndResizeTo();

    auto appBuild = QString("Built %1 %2")
            .arg(QString(APP_BUILDDATE).trimmed())
            .arg(QString(APP_BUILDTIME).trimmed());

    auto appVersion = QString("%1.%2.%3").arg(APP_VER_MAJOR).arg(APP_VER_MINOR).arg(APP_VER_PATCH);

    auto qtVersion = QString("Based on Qt %1").arg(QT_VERSION_STR);

    LayoutV({
        Space(80), // place for app title, it drawn on the background
        LayoutH({
            Stretch(),
            label(appBuild, STYLE_BUILD),
            Space(6)
        }),
        LayoutH({
            Stretch(),
            label(appVersion, STYLE_VERSION),
            Space(6)
        }),
        Stretch(),
        LayoutH({
            LayoutV({
                Stretch(),
                label(APP_COPYRIGHT, STYLE_INFO),
                label(APP_WWW, STYLE_INFO, SLOT(linkWwwClicked())),
                //label(APP_MAIL, STYLE_INFO, SLOT(linkMailClicked())) no email yet available
            }),
            Stretch(),
            LayoutV({
                Stretch(),
                label(qtVersion, STYLE_QT, SLOT(aboutQt()), qApp)
            })
        })
    })
    .setMargin(6)
    .setSpacing(0)
    .useFor(this);
}

QLabel* AboutWindow::label(const QString& text, const char* styleSheet, const char* slot, QObject* receiver)
{
    QLabel* label = slot? new Ori::Widgets::Label : new QLabel;
    label->setText(text);
    label->setStyleSheet(styleSheet);
    if (slot)
        connect(label, SIGNAL(clicked()), receiver ? receiver : this, slot);
    return label;
}

void AboutWindow::setupAutoDeletableDialog()
{
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(qApp->applicationName());
}

void AboutWindow::setBackImageAndResizeTo()
{
    QPixmap bckgnd(":/about/main");
    setMaximumSize(bckgnd.size());
    setMinimumSize(bckgnd.size());
    resize(bckgnd.size());

    auto p = palette();
    p.setBrush(QPalette::Background, QBrush(bckgnd));
    setPalette(p);
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

