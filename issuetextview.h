#ifndef ISSUETEXTVIEW_H
#define ISSUETEXTVIEW_H

#include <QTextBrowser>

class IssueTextView : public QTextBrowser
{
    Q_OBJECT

public:
    explicit IssueTextView(QWidget *parent = 0);

signals:
    void processCommand(const QString& cmd, const QUrl& url);

private:
    void linkClicked(const QUrl&);
    void linkHovered(const QUrl&);
};

QString sanitizeHtml(const QString& s);

#endif // ISSUETEXTVIEW_H
