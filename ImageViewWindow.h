#ifndef IMAGE_VIEW_WINDOW_H
#define IMAGE_VIEW_WINDOW_H

#include <QScrollArea>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class ImageViewWindow : public QScrollArea
{
    Q_OBJECT

public:
    explicit ImageViewWindow(const QFileInfo &file, QWidget *parent = 0);

    static void showImage(const QFileInfo &file, QWidget *parent = 0);

private:
    QLabel* _picture;

    void moveToScreenCenter();
};

#endif // IMAGE_VIEW_WINDOW_H
