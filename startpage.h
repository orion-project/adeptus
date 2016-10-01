#ifndef STARTPAGE_H
#define STARTPAGE_H

#include <QWidget>

namespace Adeptus {

class StartPage : public QWidget
{
    Q_OBJECT
public:
    explicit StartPage(QWidget *parent = 0);
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

#endif // STARTPAGE_H
