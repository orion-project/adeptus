#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>

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

private:
    class QLabel* label(const QString& text, const char* styleSheet,
                        const char* slot = nullptr, QObject *receiver = nullptr);
    void setupAutoDeletableDialog();
    void setBackImageAndResizeTo();
};

#endif // ABOUTWINDOW_H
