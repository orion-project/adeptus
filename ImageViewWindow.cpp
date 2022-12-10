#include "ImageViewWindow.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QScreen>

ImageViewWindow::ImageViewWindow(const QFileInfo &file, QWidget *parent) : QScrollArea(parent)
{
    _picture = new QLabel(this);
    _picture->setPixmap(QPixmap(file.filePath()));
    _picture->setToolTip(QDir::toNativeSeparators(file.filePath()));

    setWindowTitle(file.fileName());
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool);
    setBackgroundRole(QPalette::Dark);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setWidget(_picture);
}

void ImageViewWindow::moveToScreenCenter()
{
    auto windowSize =
        #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            _picture->pixmap().size()
        #else
            _picture->pixmap()->size()
        #endif
            + QSize(24, 24);
    auto screenSize = QGuiApplication::primaryScreen()->availableSize();
    if ((windowSize.width() > screenSize.width() * 0.9) ||
         windowSize.height() > screenSize.height() * 0.9)
        windowSize = screenSize * 0.9;
    resize(windowSize);

    auto awp = qApp->activeWindow()->pos();
    auto aws = qApp->activeWindow()->size();
    auto x = awp.x() + aws.width()/2 - windowSize.width()/2;
    auto y = awp.y() + aws.height()/2 - windowSize.height()/2;
    move(x, y);
}

void ImageViewWindow::showImage(const QFileInfo& file, QWidget *parent)
{
    auto w = new ImageViewWindow(file, parent);
    w->show();
    w->moveToScreenCenter();
    w->activateWindow();
}
