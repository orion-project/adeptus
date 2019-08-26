#ifndef START_PAGE_H
#define START_PAGE_H

#include <QWidget>

namespace Adeptus {

class StartPage : public QWidget
{
    Q_OBJECT
public:
    explicit StartPage(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent*);
private:
    QPixmap _background;
};


class StartPageItem : public QWidget
{
    Q_OBJECT
public:
    explicit StartPageItem(const QString& text, const QPixmap& pixmap);
};

} // namespace Adeptus

#endif // START_PAGE_H
