#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>
#include <QLabel>

#define APP_WWW "http://adeptus.orion-project.org"
#define APP_MAIL "adeptus@orion-project.org"

class AboutWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AboutWindow(QWidget *parent = 0);
protected:
    void mouseReleaseEvent(QMouseEvent*);
private slots:
    void linkWwwClicked();
    void linkMailClicked();
};

#endif // ABOUTWINDOW_H
